#include "ToyGE\RenderEngine\Effects\PostProcessVolumetricLight.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	PostProcessVolumetricLight::PostProcessVolumetricLight()
		: _density(0.8f),
		_intensity(0.8f),
		_decay(0.9f)
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"PostProcessVolumetricLight.xml");
	}

	void PostProcessVolumetricLight::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		if (!_light)
			return;

		auto sceneTex = sharedEnviroment->GetView()->GetRenderResult();
		auto linearDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		auto radialBlurInTex = Setup(sceneTex, linearDepthTex);

		auto lightPosH = _light->GetLightClipPos(sharedEnviroment->GetView()->GetCamera());
		auto lightPosUV = lightPosH * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
		
		auto volumetricLightTex = RenderVolumetricLight(radialBlurInTex, lightPosUV);

		BlurVolumetricLight(volumetricLightTex, lightPosUV, sharedEnviroment->GetView()->GetRenderResult());

		radialBlurInTex->Release();
		volumetricLightTex->Release();
	}

	Ptr<Texture> PostProcessVolumetricLight::Setup(const Ptr<Texture> & sceneTex, const Ptr<Texture> & linearDepthTex)
	{
		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(sceneTex->Desc());

		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());
		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ resultTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("Setup"));

		return resultTex;
	}

	Ptr<Texture> PostProcessVolumetricLight::RenderVolumetricLight(const Ptr<Texture> & setupTex, const float2 & lightPosUV)
	{
		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(setupTex->Desc());

		_fx->VariableByName("lightPosUV")->AsScalar()->SetValue(&lightPosUV);
		_fx->VariableByName("density")->AsScalar()->SetValue(&_density);
		_fx->VariableByName("intensity")->AsScalar()->SetValue(&_intensity);
		_fx->VariableByName("decay")->AsScalar()->SetValue(&_decay);

		_fx->VariableByName("radialBlurInTex")->AsShaderResource()->SetValue(setupTex->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets(
		{ resultTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("RadialBlur"));

		return resultTex;
	}

	void PostProcessVolumetricLight::BlurVolumetricLight(
		const Ptr<Texture> & volumetricLightTex,
		const float2 & lightPosUV,
		const Ptr<Texture> & targetTex)
	{
		_fx->VariableByName("lightPosUV")->AsScalar()->SetValue(&lightPosUV);
		_fx->VariableByName("texSize")->AsScalar()->SetValue(&targetTex->GetTexSize());

		_fx->VariableByName("volumetricLightTex")->AsShaderResource()->SetValue(volumetricLightTex->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ targetTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("BlurVolumetricLight"));
	}
}