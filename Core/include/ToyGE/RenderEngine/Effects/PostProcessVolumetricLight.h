#pragma once
#ifndef POSTPROCESSVOLUMETRICLIGHT_H
#define POSTPROCESSVOLUMETRICLIGHT_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class LightComponent;

	class TOYGE_CORE_API PostProcessVolumetricLight : public RenderAction
	{
	public:
		PostProcessVolumetricLight();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_SET(Light, Ptr<LightComponent>, _light);
		CLASS_GET(Light, Ptr<LightComponent>, _light);

		CLASS_SET(Density, float, _density);
		CLASS_GET(Density, float, _density);

		CLASS_SET(Intensity, float, _intensity);
		CLASS_GET(Intensity, float, _intensity);

		CLASS_SET(Decay, float, _decay);
		CLASS_GET(Decay, float, _decay);

	private:
		Ptr<RenderEffect> _fx;
		Ptr<LightComponent> _light;
		float _density;
		float _intensity;
		float _decay;

		Ptr<Texture> Setup(const Ptr<Texture> & sceneTex, const Ptr<Texture> & linearDepthTex);

		Ptr<Texture> RenderVolumetricLight(const Ptr<Texture> & setupTex, const float2 & lightPosUV);

		void BlurVolumetricLight(
			const Ptr<Texture> & volumetricLightTex,
			const float2 & lightPosUV,
			const Ptr<Texture> & targetTex);
	};
}

#endif