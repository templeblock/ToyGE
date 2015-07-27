#include "ToyGE\RenderEngine\Effects\HizSSR.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"

namespace ToyGE
{
	HizSSR::HizSSR()
	{
		_ssrFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"HizSSR.xml");
	}

	void HizSSR::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto rawDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::RawDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		//auto sceneTex = sharedEnviroment->ParamByName(DeferredRenderFramework::ShareName_SceneTexture())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer0 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(0))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer1 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(1))->As<SharedParam<Ptr<Texture>>>()->GetValue();

		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);
		auto sceneTex = sharedEnviroment->GetView()->GetRenderResult();
		//targetTex->CopyTo(sceneTex, 0, 0, 0, 0, 0, 0, 0);

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		RenderContextStateSave stateSave;
		rc->SaveState(
			RENDER_CONTEXT_STATE_INPUT
			| RENDER_CONTEXT_STATE_VIEWPORT
			| RENDER_CONTEXT_STATE_RENDERTARGETS
			| RENDER_CONTEXT_STATE_DEPTHSTENCIL, stateSave);
		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		auto hizTex = HiZPass(rawDepthTex);

		auto integrationTex = PreIntegration(hizTex, sharedEnviroment->GetView()->GetCamera());

		auto convolTex = Convolution(sharedEnviroment->GetView()->GetRenderResult());

		RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = static_cast<float>(sceneTex->Desc().width);
		vp.height = static_cast<float>(sceneTex->Desc().height);
		rc->SetViewport(vp);
		rc->SetDepthStencil(rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		/*auto texDesc = sceneTex->Desc();
		auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		rc->ClearRenderTargets({ tmpTex->CreateTextureView() }, { 0.0f, 0.0f, 0.0f, 0.0f });

		rc->SetRenderTargets({ sceneTex->CreateTextureView() }, 0);*/
		Tracing(gbuffer0, gbuffer1, hizTex, integrationTex, convolTex, sharedEnviroment->GetView()->GetCamera(), sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView());

		sharedEnviroment->GetView()->FlipRenderTarget();

		rc->RestoreState(stateSave);

		hizTex->Release();
		integrationTex->Release();
		convolTex->Release();
		//sceneTex->Release();

		/*rc->SetDepthStencil(ResourceView());
		auto backFrameBuffer = Global::GetRenderEngine()->DefualtRenderTarget();
		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ backFrameBuffer->CreateTextureView() }, 0);
		DebugQuad(tmpTex->CreateTextureView(0, 1, 0, 1), COLOR_WRITE_ALL);*/
	}

	Ptr<Texture> HizSSR::HiZPass(const Ptr<Texture> & rawDepthTex)
	{
		auto texDesc = rawDepthTex->Desc();
		texDesc.mipLevels = 0;
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		auto tex0 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto tex1 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = static_cast<float>(rawDepthTex->Desc().width);
		vp.height = static_cast<float>(rawDepthTex->Desc().height);
		rc->SetViewport(vp);

		_ssrFX->VariableByName("rawDepthTex")->AsShaderResource()->SetValue(rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
		rc->SetRenderTargets({ tex0->CreateTextureView(0, 1, 0, 1) }, 0);
		_ssrFX->TechniqueByName("HiZ_Initial")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_ssrFX->TechniqueByName("HiZ_Initial")->PassByIndex(0)->UnBind();

		int32_t preWidth = texDesc.width;
		int32_t preHeight = texDesc.height;
		int32_t width = std::max<int32_t>(1, texDesc.width >> 1);
		int32_t height = std::max<int32_t>(1, texDesc.height >> 1);
		int32_t mipLevel = 1;

		while (true)
		{
			vp.width = static_cast<float>(width);
			vp.height = static_cast<float>(height);
			rc->SetViewport(vp);

			float2 preTexSize = float2(static_cast<float>(preWidth), static_cast<float>(preHeight));
			_ssrFX->VariableByName("preMipTexSize")->AsScalar()->SetValue(&preTexSize, sizeof(preTexSize));
			int32_t preMipLevel = static_cast<int32_t>(mipLevel - 1);
			_ssrFX->VariableByName("preMipLevel")->AsScalar()->SetValue(&preMipLevel, sizeof(preMipLevel));
			_ssrFX->VariableByName("hiZTex")->AsShaderResource()->SetValue(tex0->CreateTextureView(0, 0, 0, 1));
			rc->SetRenderTargets({ tex1->CreateTextureView(mipLevel, 1, 0, 1) }, 0);
			_ssrFX->TechniqueByName("HiZ")->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			_ssrFX->TechniqueByName("HiZ")->PassByIndex(0)->UnBind();

			tex1->CopyTo(tex0, mipLevel, 0, 0, 0, 0, mipLevel, 0);
			++mipLevel;

			if (width == 1 && height == 1)
				break;

			preWidth = width;
			preHeight = height;
			width = std::max<int>(1, width >> 1);
			height = std::max<int>(1, height >> 1);
		}

		tex1->Release();

		return tex0;
	}

	Ptr<Texture> HizSSR::PreIntegration(const Ptr<Texture> & hizTex, const Ptr<Camera> & camera)
	{
		auto texDesc = hizTex->Desc();
		texDesc.format = RENDER_FORMAT_R8_UNORM;
		auto tex0 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto tex1 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->ClearRenderTargets({ tex0->CreateTextureView() }, 1.0f);

		RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		int32_t preWidth = texDesc.width;
		int32_t preHeight = texDesc.height;
		int32_t width = std::max<int>(1, texDesc.width >> 1);
		int32_t height = std::max<int>(1, texDesc.height >> 1);
		int32_t mipLevel = 1;

		float projA = camera->ProjMatrix()._33;
		_ssrFX->VariableByName("projA")->AsScalar()->SetValue(&projA);
		float projB = camera->ProjMatrix()._43;
		_ssrFX->VariableByName("projB")->AsScalar()->SetValue(&projB);
		float2 cameraNearFar = float2(camera->Near(), camera->Far());
		_ssrFX->VariableByName("cameraNearFar")->AsScalar()->SetValue(&cameraNearFar);

		_ssrFX->VariableByName("hiZTex")->AsShaderResource()->SetValue(hizTex->CreateTextureView(0, 0, 0, 1));

		while (true)
		{
			vp.width = static_cast<float>(width);
			vp.height = static_cast<float>(height);
			rc->SetViewport(vp);

			float2 preMipSize = float2(static_cast<float>(preWidth), static_cast<float>(preHeight));
			_ssrFX->VariableByName("preMipTexSize")->AsScalar()->SetValue(&preMipSize, sizeof(preMipSize));
			int32_t preMipLevel = static_cast<int32_t>(mipLevel - 1);
			_ssrFX->VariableByName("preMipLevel")->AsScalar()->SetValue(&preMipLevel, sizeof(preMipLevel));

			_ssrFX->VariableByName("visibilityTex")->AsShaderResource()->SetValue(tex0->CreateTextureView(0, 0, 0, 1));
			rc->SetRenderTargets({ tex1->CreateTextureView(mipLevel, 1, 0, 1) }, 0);
			_ssrFX->TechniqueByName("PreIntegration")->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			_ssrFX->TechniqueByName("PreIntegration")->PassByIndex(0)->UnBind();

			tex1->CopyTo(tex0, mipLevel, 0, 0, 0, 0, mipLevel, 0);
			++mipLevel;

			if (width == 1 && height == 1)
				break;

			preWidth = width;
			preHeight = height;
			width = std::max<int>(1, width >> 1);
			height = std::max<int>(1, height >> 1);
		}

		tex1->Release();

		return tex0;
	}

	Ptr<Texture> HizSSR::Convolution(const Ptr<Texture> & sceneTex)
	{
		auto texDesc = sceneTex->Desc();
		texDesc.mipLevels = 0;

		auto tex0 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto tex1 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		sceneTex->CopyTo(tex0, 0, 0, 0, 0, 0, 0, 0);

		RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		int32_t preWidth = texDesc.width;
		int32_t preHeight = texDesc.height;
		int32_t width = std::max<int>(1, texDesc.width >> 1);
		int32_t height = std::max<int>(1, texDesc.height >> 1);
		int32_t mipLevel = 1;

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto gaussTableVar = _ssrFX->VariableByName("gaussTable")->AsScalar();
		int blurRadius = 3;
		for (int i = 0; i <= blurRadius; ++i)
		{
			gaussTableVar->SetValue(&Blur::GaussTable(blurRadius)[i], sizeof(float), sizeof(float) * i);
		}

		while (true)
		{
			vp.width = static_cast<float>(width);
			vp.height = static_cast<float>(height);
			rc->SetViewport(vp);

			float2 preMipSize = float2(static_cast<float>(preWidth), static_cast<float>(preHeight));
			_ssrFX->VariableByName("preMipTexSize")->AsScalar()->SetValue(&preMipSize, sizeof(preMipSize));
			int32_t preMipLevel = static_cast<int32_t>(mipLevel - 1);
			_ssrFX->VariableByName("preMipLevel")->AsScalar()->SetValue(&preMipLevel, sizeof(preMipLevel));

			_ssrFX->VariableByName("convolutionTex")->AsShaderResource()->SetValue(tex0->CreateTextureView(0, 0, 0, 1));
			rc->SetRenderTargets({ tex1->CreateTextureView(mipLevel, 1, 0, 1) }, 0);
			_ssrFX->TechniqueByName("GaussConvolutionX")->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			_ssrFX->TechniqueByName("GaussConvolutionX")->PassByIndex(0)->UnBind();

			float2 curMipSize = float2(static_cast<float>(width), static_cast<float>(height));
			_ssrFX->VariableByName("curMipTexSize")->AsScalar()->SetValue(&curMipSize, sizeof(curMipSize));
			
			_ssrFX->VariableByName("convolutionTex")->AsShaderResource()->SetValue(tex1->CreateTextureView(0, 0, 0, 1));
			rc->SetRenderTargets({ tex0->CreateTextureView(mipLevel, 1, 0, 1) }, 0);
			_ssrFX->TechniqueByName("GaussConvolutionY")->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			_ssrFX->TechniqueByName("GaussConvolutionY")->PassByIndex(0)->UnBind();

			++mipLevel;

			if (width == 1 && height == 1)
				break;

			preWidth = width;
			preHeight = height;
			width = std::max<int>(1, width >> 1);
			height = std::max<int>(1, height >> 1);
		}

		tex1->Release();

		return tex0;
	}

	void HizSSR::Tracing(
		const Ptr<Texture> & gbuffer0,
		const Ptr<Texture> & gbuffer1,
		const Ptr<Texture> & hizTex,
		const Ptr<Texture> & integrationTex,
		const Ptr<Texture> & convolTex,
		const Ptr<Camera> & camera,
		const ResourceView & target)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto & proj = camera->ProjMatrix();
		_ssrFX->VariableByName("proj")->AsScalar()->SetValue(&proj);
		auto projXM = XMLoadFloat4x4(&proj);
		auto invProjXM = XMMatrixInverse(&XMMatrixDeterminant(projXM), projXM);
		XMFLOAT4X4 invProj;
		XMStoreFloat4x4(&invProj, invProjXM);
		_ssrFX->VariableByName("invProj")->AsScalar()->SetValue(&invProj);
		_ssrFX->VariableByName("view")->AsScalar()->SetValue(&camera->ViewMatrix());

		_ssrFX->VariableByName("gbuffer0")->AsShaderResource()->SetValue(gbuffer0->CreateTextureView());
		_ssrFX->VariableByName("gbuffer1")->AsShaderResource()->SetValue(gbuffer1->CreateTextureView());
		_ssrFX->VariableByName("hiZTex")->AsShaderResource()->SetValue(hizTex->CreateTextureView(0, 0, 0, 1));
		_ssrFX->VariableByName("visibilityTex")->AsShaderResource()->SetValue(integrationTex->CreateTextureView(0, 0, 0, 1));
		_ssrFX->VariableByName("convolutionTex")->AsShaderResource()->SetValue(convolTex->CreateTextureView(0, 0, 0, 1));

		rc->SetRenderTargets({ target }, 0);

		_ssrFX->TechniqueByName("Tracing")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_ssrFX->TechniqueByName("Tracing")->PassByIndex(0)->UnBind();
	}
}