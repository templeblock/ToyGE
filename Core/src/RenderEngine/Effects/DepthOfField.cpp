#include "ToyGE\RenderEngine\Effects\DepthOfField.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"

namespace ToyGE
{
	DepthOfField::DepthOfField()
	{
		_dofFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"DOF.xml");
	}

	void DepthOfField::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		//auto sceneTex = sharedEnviroment->ParamByName(DeferredRenderFramework::ShareName_SceneTexture())->As<SharedParamTexture>()->Texture();
		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);
		//auto sceneTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(targetTex->Desc());
		//targetTex->CopyTo(sceneTex, 0, 0, 0, 0, 0, 0, 0);

		auto linearDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		RenderContextStateSave stateSave;
		rc->SaveState(
			RENDER_CONTEXT_STATE_DEPTHSTENCIL
			| RENDER_CONTEXT_STATE_INPUT
			| RENDER_CONTEXT_STATE_RENDERTARGETS
			| RENDER_CONTEXT_STATE_VIEWPORT, stateSave);

		rc->SetDepthStencil(ResourceView());
		rc->SetRenderInput(CommonInput::QuadInput());

		auto packTex = ComputeCoC(sharedEnviroment->GetView()->GetRenderResult(), linearDepthTex, sharedEnviroment->GetView()->GetCamera());
		auto blurResult = Blur(packTex);
		Composite(packTex, blurResult.first, blurResult.second, sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView());

		sharedEnviroment->GetView()->FlipRenderTarget();

		rc->RestoreState(stateSave);

		packTex->Release();
		blurResult.first->Release();
		blurResult.second->Release();

		//sharedEnviroment->SetParam("finalTex", std::make_shared<SharedParamTexture>(_compositeTex));

		//rc->SetDepthStencil(ResourceView());
		//auto backFrameBuffer = Global::GetRenderEngine()->DefualtRenderTarget();
		//Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ backFrameBuffer->CreateTextureView() }, 0);
		//DebugQuad(_nearFieldTex->CreateTextureView(0, 1, 0, 1), COLOR_WRITE_ALL);
	}

	Ptr<Texture> DepthOfField::ComputeCoC(const Ptr<Texture> & sceneTex, const Ptr<Texture> & linearDepthTex, const Ptr<Camera> & camera)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = sceneTex->Desc();
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;

		auto packTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		//auto camera = Global::GetRenderEngine()->Camera();
		float camFar = camera->Far();

		const float maxCoC = 10.0f;
		const float nearStart = 0.1f / camera->Far();
		const float nearEnd = 2.0f / camera->Far();
		const float farStart = 3.0f / camera->Far();
		const float farEnd = 10.0f / camera->Far();

		_dofFX->VariableByName("maxCoC")->AsScalar()->SetValue(&maxCoC);
		_dofFX->VariableByName("nearStart")->AsScalar()->SetValue(&nearStart);
		_dofFX->VariableByName("nearEnd")->AsScalar()->SetValue(&nearEnd);
		_dofFX->VariableByName("farStart")->AsScalar()->SetValue(&farStart);
		_dofFX->VariableByName("farEnd")->AsScalar()->SetValue(&farEnd);

		_dofFX->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());
		_dofFX->VariableByName("depthLinearTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());
		rc->SetRenderTargets({ packTex->CreateTextureView() }, 0);

		_dofFX->TechniqueByName("ComputeCoC")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_dofFX->TechniqueByName("ComputeCoC")->PassByIndex(0)->UnBind();

		return packTex;
	}

	std::pair<Ptr<Texture>, Ptr<Texture>> DepthOfField::Blur(const Ptr<Texture> & packTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = packTex->Desc();
		texDesc.width /= 2;
		
		auto nearFieldTexTmp = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto blurTexTmp = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		texDesc.height /= 2;

		auto nearFieldTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto blurTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto preVp = rc->GetViewport();
		

		auto gaussTableVar = _dofFX->VariableByName("gaussTable")->AsScalar();
		uint32_t blurRadius = 6;
		for (uint32_t i = 0; i <= blurRadius; ++i)
		{
			gaussTableVar->SetValue(&Blur::GaussTable(blurRadius)[i], sizeof(float), sizeof(float) * i);
		}

		//BlurX
		RenderViewport vp = preVp;
		vp.width = static_cast<float>(packTex->Desc().width / 2);
		vp.height = static_cast<float>(packTex->Desc().height);
		rc->SetViewport(vp);

		float2 quadSize = float2(vp.width, vp.height);
		_dofFX->VariableByName("quadSize")->AsScalar()->SetValue(&quadSize);

		_dofFX->VariableByName("blurInputTex")->AsShaderResource()->SetValue(packTex->CreateTextureView());
		rc->SetRenderTargets({ nearFieldTexTmp->CreateTextureView(), blurTexTmp->CreateTextureView() }, 0);
		_dofFX->TechniqueByName("BlurX")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_dofFX->TechniqueByName("BlurX")->PassByIndex(0)->UnBind();

		//BlurY
		vp.width = static_cast<float>(packTex->Desc().width / 2);
		vp.height = static_cast<float>(packTex->Desc().height / 2);
		rc->SetViewport(vp);

		quadSize = float2(vp.width, vp.height);
		_dofFX->VariableByName("quadSize")->AsScalar()->SetValue(&quadSize);

		_dofFX->VariableByName("blurInputTex")->AsShaderResource()->SetValue(blurTexTmp->CreateTextureView());
		_dofFX->VariableByName("nearFieldTex")->AsShaderResource()->SetValue(nearFieldTexTmp->CreateTextureView());
		rc->SetRenderTargets({ nearFieldTex->CreateTextureView(), blurTex->CreateTextureView() }, 0);
		_dofFX->TechniqueByName("BlurY")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_dofFX->TechniqueByName("BlurY")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVp);

		nearFieldTexTmp->Release();
		blurTexTmp->Release();

		return std::make_pair(nearFieldTex, blurTex);
	}

	void DepthOfField::Composite(const Ptr<Texture> & packTex, const Ptr<Texture> & nearFieldTex, const Ptr<Texture> & blurTex, const ResourceView & target)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = packTex->Desc();
		//_compositeTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

		_dofFX->VariableByName("packTex")->AsShaderResource()->SetValue(packTex->CreateTextureView());
		_dofFX->VariableByName("blurTex")->AsShaderResource()->SetValue(blurTex->CreateTextureView());
		rc->SetRenderTargets({ target }, 0);
		_dofFX->TechniqueByName("Composite")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_dofFX->TechniqueByName("Composite")->PassByIndex(0)->UnBind();
	}
}