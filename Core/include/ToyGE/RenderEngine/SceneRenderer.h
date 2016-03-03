#pragma once
#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class RenderView;

	class TOYGE_CORE_API SceneRenderer
	{
	public:
		static const int32_t temporalAANumSamples = 8;

		bool bGenVelocityMap = false;
		bool bRenderingAtmosphere = false;
		bool bOIT = true;
		bool bTAA = true;
		bool bLPV = false;

		bool bSSR = true;
		float ssrMaxRoughness = 0.8f;
		float ssrIntensity = 1.0f;

		SceneRenderer();

		virtual ~SceneRenderer() = default;

		void SetSunDirection(const float3 & sunDirection);

		void SetSunRadiance(const float3 & sunRadiance);

		void SetSunRenderRadius(float sunRenderRadius);

		void SetSunLight(const Ptr<class DirectionalLightComponent> & light);
		Ptr<class DirectionalLightComponent> GetSunLight() const
		{
			return _sunLight;
		}

		void UpdateSunLight();

		virtual void Render(const Ptr<RenderView> & view) = 0;

		Ptr<class LPV> GetLPVRenderer() const
		{
			return _lpv;
		}

		Ptr<class VolumetricLight> GetVolumetricLightingRenderer() const
		{
			return _volumetricLight;
		}

	protected:
		float3 _sunDirection;
		float3 _sunRadiance;
		float _sunRenderRadius;
		Ptr<class DirectionalLightComponent> _sunLight;

		Ptr<class AtmosphereRendering> _atmosphereRendering;
		Ptr<class LPV> _lpv;
		Ptr<class VolumetricLight> _volumetricLight;
		Ptr<class SSR> _ssrRenderer;
	};
}

#endif