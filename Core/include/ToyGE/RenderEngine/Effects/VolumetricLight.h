#pragma once
#ifndef VOLUMETRICLIGHT_H
#define VOLUMETRICLIGHT_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	class RenderComponent;
	class Camera;

	class TOYGE_CORE_API VolumetricLight : public RenderAction
	{
	public:
		VolumetricLight();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_GET(Attenuation, float, _attenuation);
		CLASS_SET(Attenuation, float, _attenuation);

		CLASS_GET(Scattering, float, _scattering);
		CLASS_SET(Scattering, float, _scattering);

		CLASS_GET(PhaseFunctionParam, float, _phaseFunctionParam);
		CLASS_SET(PhaseFunctionParam, float, _phaseFunctionParam);

	private:
		Ptr<RenderEffect> _fx;
		Ptr<RenderComponent> _pointLightVolumeGeo;
		Ptr<RenderComponent> _spotLightVolumeGeo;
		float _attenuation;
		float _scattering;
		float _phaseFunctionParam;

		void InitDither();

		void RenderPointLightVolume(
			const Ptr<PointLightComponent> & light,
			const Ptr<Camera> & camera,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & lightVolumeTex);

		void RenderSpotLightVolume(
			const Ptr<SpotLightComponent> & light,
			const Ptr<Camera> & camera,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & lightVolumeTex);

		void RenderDirectionalLightVolume(
			const Ptr<DirectionalLightComponent> & light,
			const Ptr<Camera> & camera,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & lightVolumeTex);

		Ptr<Texture> BilateralGaussBlur(const Ptr<Texture> & lightVolumeTex, const Ptr<Texture> & linearDepthTex);

		void BilateralUpSampling(
			const Ptr<Texture> & lightVolumeTex,
			const Ptr<Texture> & highResLinearDepthTex,
			const Ptr<Texture> & lowResLinearDepthTex,
			const ResourceView & target);
	};
}

#endif