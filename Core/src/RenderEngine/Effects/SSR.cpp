#include "ToyGE\RenderEngine\Effects\SSR.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\Kernel\GlobalInfo.h"

namespace ToyGE
{
	SSR::SSR()
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"SSR.xml");

		//InitDither();
	}

	void SSR::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		//auto linearDepthTex = sharedEnviroment->ParamByName(DeferredRenderFramework::ShareName_LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto rawDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::RawDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer0 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(0))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer1 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(1))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);
		auto sceneTex = sharedEnviroment->GetView()->GetRenderResult();//Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(targetTex->Desc());
		//targetTex->CopyTo(sceneTex, 0, 0, 0, 0, 0, 0, 0);
		//sceneTex->GenerateMips();
		sceneTex->CopyTo(sharedEnviroment->GetView()->GetRenderTarget(), 0, 0, 0, 0, 0, 0, 0);

		auto hzb = BuildHZB(rawDepthTex);

		auto camera = sharedEnviroment->GetView()->GetCamera();

		_fx->VariableByName("view")->AsScalar()->SetValue(&camera->ViewMatrix());
		_fx->VariableByName("proj")->AsScalar()->SetValue(&camera->ProjMatrix());
		float2 cameraNearFar = float2(camera->Near(), camera->Far());
		_fx->VariableByName("cameraNearFar")->AsScalar()->SetValue(&cameraNearFar);
		float w = static_cast<float>(sceneTex->Desc().width);
		float h = static_cast<float>(sceneTex->Desc().height);
		float4 screenSize = float4(w, h, 1.0f / w, 1.0f / h);
		_fx->VariableByName("screenSize")->AsScalar()->SetValue(&screenSize, sizeof(screenSize));

		auto frameCount = Global::GetInfo()->GetFrameCount();
		_fx->VariableByName("frameCount")->AsScalar()->SetValue(&frameCount);

		_fx->VariableByName("depthTex")->AsShaderResource()->SetValue(hzb->CreateTextureView(0, 0));
		_fx->VariableByName("gbuffer0")->AsShaderResource()->SetValue(gbuffer0->CreateTextureView());
		_fx->VariableByName("gbuffer1")->AsShaderResource()->SetValue(gbuffer1->CreateTextureView());
		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		rc->SetRenderTargets({ sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView() }, 0);

		RenderQuad(
			_fx->TechniqueByName("ScreenSpaceReflection"),
			0, 0,
			sceneTex->Desc().width, sceneTex->Desc().height,
			0.0f, 0.0f, 1.0f, 1.0f,
			rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		sharedEnviroment->GetView()->FlipRenderTarget();

		hzb->Release();
		//sceneTex->Release();
	}

	void SSR::InitDither()
	{
		float data[] =
		{
			0.0f, 8.0f, 2.0f, 10.0f,
			12.0f, 4.0f, 14.0f, 6.0f,
			3.0f, 11.0f, 1.0f, 9.0f,
			15.0f, 7.0f, 13.0f, 5.0f
		};
		for (auto & i : data)
		{
			i /= 16.0f;
		}

		_fx->VariableByName("dither")->AsScalar()->SetValue(data);
	}

	Ptr<Texture> SSR::BuildHZB(const Ptr<Texture> & depthTex)
	{
		auto texDesc = depthTex->Desc();
		texDesc.format = RENDER_FORMAT_R16_FLOAT;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.mipLevels = 0;

		auto hzb0 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto hzb1 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		//depthTex->CopyTo(hzb0, 0, 0, 0, 0, 0, 0, 0);
		Transform(depthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_R24_UNORM_X8_TYPELESS), hzb0->CreateTextureView(), COLOR_WRITE_R);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		for (int32_t mipLevel = 1; mipLevel < hzb0->Desc().mipLevels; ++mipLevel)
		{
			auto & mipSize = hzb0->GetMipSize(mipLevel - 1);
			auto w = static_cast<float>(std::get<0>(mipSize));
			auto h = static_cast<float>(std::get<1>(mipSize));
			float4 screenSize = float4(w, h, 1.0f / w, 1.0f / h);
			_fx->VariableByName("screenSize")->AsScalar()->SetValue(&screenSize);

			uint32_t level = static_cast<uint32_t>(mipLevel - 1);
			_fx->VariableByName("mipLevel")->AsScalar()->SetValue(&level);

			_fx->VariableByName("depthTex")->AsShaderResource()->SetValue(hzb0->CreateTextureView(0, 0));

			rc->SetRenderTargets({ hzb1->CreateTextureView(mipLevel, 1) }, 0);

			auto & targetMipSize = hzb0->GetMipSize(mipLevel);

			RenderQuad(_fx->TechniqueByName("HZBBuild"), 0, 0, std::get<0>(targetMipSize), std::get<1>(targetMipSize));

			hzb1->CopyTo(hzb0, mipLevel, 0, 0, 0, 0, mipLevel, 0);
		}

		hzb1->Release();

		return hzb0;
	}
}