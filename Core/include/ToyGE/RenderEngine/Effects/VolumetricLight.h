#pragma once
#ifndef VOLUMETRICLIGHT_H
#define VOLUMETRICLIGHT_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	DECLARE_SHADER(, RenderLightVolumeVS, SHADER_VS, "VolumetricLightVS", "RenderLightVolumeVS", SM_4);
	DECLARE_SHADER(, RenderPointLightVolumePS, SHADER_PS, "VolumetricLight", "RenderPointLightVolumePS", SM_4);
	DECLARE_SHADER(, RenderSpotLightVolumePS, SHADER_PS, "VolumetricLight", "RenderSpotLightVolumePS", SM_4);
	DECLARE_SHADER(, RenderDirectionalLightVolumePS, SHADER_PS, "VolumetricLight", "RenderDirectionalLightVolumePS", SM_4);

	class TOYGE_CORE_API VolumetricLight : public RenderAction
	{
	public:
		VolumetricLight();

		void Render(const Ptr<RenderView> & view) override;

		CLASS_GET(Attenuation, float, _attenuation);
		CLASS_SET(Attenuation, float, _attenuation);

		CLASS_GET(Scattering, float, _scattering);
		CLASS_SET(Scattering, float, _scattering);

		CLASS_GET(PhaseFunctionParam, float, _phaseFunctionParam);
		CLASS_SET(PhaseFunctionParam, float, _phaseFunctionParam);

	private:
		Ptr<class RenderMeshComponent> _pointLightVolumeGeo;
		Ptr<class RenderMeshComponent> _spotLightVolumeGeo;
		Ptr<RenderBuffer> _ditherBuffer;
		float _attenuation;
		float _scattering;
		float _phaseFunctionParam;

		void InitDither();

		void RenderPointLightVolume(
			const Ptr<PointLightComponent> & light,
			const Ptr<RenderView> & view,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & lightVolumeTex);

		void RenderSpotLightVolume(
			const Ptr<SpotLightComponent> & light,
			const Ptr<RenderView> & view,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & lightVolumeTex);

		void RenderDirectionalLightVolume(
			const Ptr<DirectionalLightComponent> & light,
			const Ptr<RenderView> & view,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & lightVolumeTex);
	};
}

#endif