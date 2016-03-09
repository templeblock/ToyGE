#include "ToyGE\RenderEngine\Effects\PostProcessVolumetricLight.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	PostProcessVolumetricLight::PostProcessVolumetricLight()
		: _density(0.8f),
		_intensity(0.8f),
		_decay(0.7f)
	{
	}

	void PostProcessVolumetricLight::Render(const Ptr<RenderView> & view)
	{
		if (!_light)
			return;

		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
		auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");

		auto radialBlurInTexRef = Setup(sceneTex, sceneLinearClipDepth);
		auto radialBlurInTex = radialBlurInTexRef->Get()->Cast<Texture>();

		auto lightPosH = _light->GetClipSpacePos(view->GetCamera());
		auto lightPosUV = lightPosH * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
		
		auto volumetricLightTexRef = RenderVolumetricLight(lightPosUV, radialBlurInTex);
		auto volumetricLightTex = volumetricLightTexRef->Get()->Cast<Texture>();

		BlurVolumetricLight(lightPosUV, sceneLinearClipDepth, volumetricLightTex, sceneTex->GetRenderTargetView(0, 0, 1));
	}

	PooledTextureRef PostProcessVolumetricLight::Setup(const Ptr<Texture> & sceneTex, const Ptr<Texture> & linearDepthTex)
	{
		auto texDesc = sceneTex->GetDesc();
		texDesc.width /= 2;
		texDesc.height /= 2;
		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto resultTex = resultTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<PPVolumeSetupPS>();

		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		ps->SetSRV("linearDepthTex", linearDepthTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ resultTex->GetRenderTargetView(0, 0, 1) });

		return resultTexRef;
	}

	PooledTextureRef PostProcessVolumetricLight::RenderVolumetricLight(const float2 & lightPosUV, const Ptr<Texture> & setupTex)
	{
		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, setupTex->GetDesc() });
		auto resultTex = resultTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<RadialBlurPS>();

		ps->SetScalar("lightPosUV", lightPosUV);
		ps->SetScalar("density", _density);
		ps->SetScalar("intensity", _intensity);
		ps->SetScalar("decay", _decay);
		ps->SetScalar("frameCount", (uint32_t)Global::GetInfo()->frameCount);

		ps->SetSRV("sceneTex", setupTex->GetShaderResourceView());

		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ resultTex->GetRenderTargetView(0, 0, 1) });

		return resultTexRef;
	}

	void PostProcessVolumetricLight::BlurVolumetricLight(
		const float2 & lightPosUV,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & volumetricLightTex,
		const Ptr<RenderTargetView> & target)
	{
		auto ps = Shader::FindOrCreate<BlurVolumetricLightPS>();

		ps->SetScalar("lightPosUV", lightPosUV);
		ps->SetScalar("texSize", volumetricLightTex->GetTexSize());

		ps->SetSRV("volumetricLightTex", volumetricLightTex->GetShaderResourceView());
		ps->SetSRV("linearDepthTex", linearDepthTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

		DrawQuad({ target });

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);
	}
}