#include "ToyGE\RenderEngine\Effects\SubsurfaceScattering.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	SubsurfaceSacttering::SubsurfaceSacttering()
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"SubSurfaceScattering.xml");
	}

	void SubsurfaceSacttering::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		_fx->SetExtraMacros({});

		auto rawDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::RawDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto linearDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer0 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(0))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer1 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(1))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto lighting0 = sharedEnviroment->ParamByName(CommonRenderShareName::Lighting(0))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto lighting1 = sharedEnviroment->ParamByName(CommonRenderShareName::Lighting(1))->As<SharedParam<Ptr<Texture>>>()->GetValue();

		sharedEnviroment->GetView()->BindParams(_fx);

		auto diffuseTex = ComputeDiffuse(rawDepthTex, gbuffer0, gbuffer1, lighting0);

		Transmittance(
			sharedEnviroment->GetView()->GetRenderLights(),
			sharedEnviroment->GetView()->GetCamera(),
			rawDepthTex,
			diffuseTex);

		auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(lighting0->Desc());
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());
		_fx->VariableByName("lighting1")->AsShaderResource()->SetValue(lighting1->CreateTextureView());
		_fx->VariableByName("diffuseTex")->AsShaderResource()->SetValue(diffuseTex->CreateTextureView());

		//BlurX
		rc->SetRenderTargets({ tmpTex->CreateTextureView() }, 0);
		rc->ClearRenderTargets(0.0f);

		RenderQuad(
			_fx->TechniqueByName("SubsurfaceScatteingBlurX"),
			0, 0, 0, 0,
			0.0f, 0.0f, 1.0f, 1.0f,
			rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		//BlurY
		_fx->AddExtraMacro("ADD_SPECULAR", "");
		_fx->UpdateData();

		_fx->VariableByName("diffuseTex")->AsShaderResource()->SetValue(tmpTex->CreateTextureView());

		rc->SetRenderTargets({ sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView() }, 0);

		RenderQuad(
			_fx->TechniqueByName("SubsurfaceScatteingBlurY"),
			0, 0, 0, 0,
			0.0f, 0.0f, 1.0f, 1.0f,
			rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));


		diffuseTex->Release();
		tmpTex->Release();
	}

	Ptr<Texture> SubsurfaceSacttering::ComputeDiffuse(
		const Ptr<Texture> & rawDepthTex,
		const Ptr<Texture> & gbuffer0,
		const Ptr<Texture> & gbuffer1,
		const Ptr<Texture> & lighting0)
	{
		auto diffuseTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(lighting0->Desc());

		_fx->VariableByName("gbuffer0")->AsShaderResource()->SetValue(gbuffer0->CreateTextureView());
		_fx->VariableByName("gbuffer1")->AsShaderResource()->SetValue(gbuffer1->CreateTextureView());
		_fx->VariableByName("lighting0")->AsShaderResource()->SetValue(lighting0->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ diffuseTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("ComputeDiffuse"),
			0, 0, 0, 0,
			0.0f, 0.0f, 1.0f, 1.0f, 
			rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		return diffuseTex;
	}

	void SubsurfaceSacttering::Transmittance(
		const std::vector<Ptr<LightComponent>> & lights,
		const Ptr<Camera> & camera,
		const Ptr<Texture> & rawDepthTex,
		const Ptr<Texture> & targetTex)
	{
		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ targetTex->CreateTextureView() }, 0);

		for (auto & light : lights)
		{
			if (!light->IsCastShadow())
				continue;

			light->BindMacros(_fx, false, camera);
			_fx->UpdateData();
			light->BindParams(_fx, false, camera);

			RenderQuad(_fx->TechniqueByName("Transmittance"),
				0, 0, 0, 0,
				0.0f, 0.0f, 1.0f, 1.0f,
				rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
		}
	}
}