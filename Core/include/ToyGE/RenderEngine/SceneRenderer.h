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
		bool bGenVelocityMap = false;
		bool bRenderingAtmosphere = false;
		bool bOIT = true;

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

	protected:
		float3 _sunDirection;
		float3 _sunRadiance;
		float _sunRenderRadius;
		Ptr<class DirectionalLightComponent> _sunLight;

		Ptr<class AtmosphereRendering> _atmosphereRendering;

	};
}

#endif