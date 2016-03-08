#pragma once
#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class RenderView;

	class SceneRenderingConfig
	{
	public:
		bool bGenVelocityMap = false;
		bool bOIT = true;
		bool bTAA = true;
		bool bLPV = false;
		bool bRenderEnvReflection = true;
		bool bReflectAmbientMap = true;
		bool bRenderAmbient = true;

		bool bSSR = true;
		float ssrMaxRoughness = 0.8f;
		float ssrIntensity = 1.0f;
	};

	class TOYGE_CORE_API SceneRenderer
	{
	public:
		SceneRenderer();

		virtual ~SceneRenderer() = default;

		Ptr<class AtmosphereRendering> GetAtmosphereRenderer() const
		{
			return _atmosphereRendering;
		}

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
		Ptr<class AtmosphereRendering> _atmosphereRendering;
		Ptr<class LPV> _lpv;
		Ptr<class VolumetricLight> _volumetricLight;
		Ptr<class SSR> _ssrRenderer;
		Ptr<class TranslucentRendering> _translucentRendering;
		Ptr<class EnvironmentReflectionRenderer> _envReflectionRenderer;
		Ptr<class EyeAdaption> _eyeAdaption;
	};
}

#endif