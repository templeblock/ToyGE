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
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"PostProcessVolumetricLight.xml");
	}

	void PostProcessVolumetricLight::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		if (!_light)
			return;

		auto sceneTex = sharedEnviroment->GetView()->GetRenderResult();
		auto linearDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		auto radialBlurInTex = Startup(sceneTex, linearDepthTex);
		_fx->VariableByName("radialBlurInTex")->AsShaderResource()->SetValue(radialBlurInTex->CreateTextureView());

		auto lightPosH = _light->GetLightClipPos(sharedEnviroment->GetView()->GetCamera());
		auto lightPosUV = lightPosH * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
		_fx->VariableByName("lightPosUV")->AsScalar()->SetValue(&lightPosUV);

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets(
		{ sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("RadialBlur"));

		radialBlurInTex->Release();
	}

	Ptr<Texture> PostProcessVolumetricLight::Startup(const Ptr<Texture> & sceneTex, const Ptr<Texture> & linearDepthTex)
	{
		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(sceneTex->Desc());

		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());
		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ resultTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("Startup"));

		return resultTex;
	}
}