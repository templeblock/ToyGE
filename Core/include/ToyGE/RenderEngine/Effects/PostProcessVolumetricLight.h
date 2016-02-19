#pragma once
#ifndef POSTPROCESSVOLUMETRICLIGHT_H
#define POSTPROCESSVOLUMETRICLIGHT_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	DECLARE_SHADER(, PPVolumeSetupPS, SHADER_PS, "PostProcessVolumetricLight", "PPVolumeSetupPS", SM_4);
	DECLARE_SHADER(, RadialBlurPS, SHADER_PS, "PostProcessVolumetricLight", "RadialBlurPS", SM_4);
	DECLARE_SHADER(, BlurVolumetricLightPS, SHADER_PS, "PostProcessVolumetricLight", "BlurVolumetricLightPS", SM_4);

	class LightComponent;

	class TOYGE_CORE_API PostProcessVolumetricLight : public RenderAction
	{
	public:
		PostProcessVolumetricLight();

		virtual void Render(const Ptr<RenderView> & view) override;

		CLASS_SET(Light, Ptr<LightComponent>, _light);
		CLASS_GET(Light, Ptr<LightComponent>, _light);

		CLASS_SET(Density, float, _density);
		CLASS_GET(Density, float, _density);

		CLASS_SET(Intensity, float, _intensity);
		CLASS_GET(Intensity, float, _intensity);

		CLASS_SET(Decay, float, _decay);
		CLASS_GET(Decay, float, _decay);

	private:
		Ptr<LightComponent> _light;
		float _density;
		float _intensity;
		float _decay;

		PooledTextureRef Setup(const Ptr<Texture> & sceneTex, const Ptr<Texture> & linearDepthTex);

		PooledTextureRef RenderVolumetricLight(const float2 & lightPosUV, const Ptr<Texture> & setupTex);

		void BlurVolumetricLight(
			const float2 & lightPosUV,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & volumetricLightTex,
			const Ptr<RenderTargetView> & target);
	};
}

#endif