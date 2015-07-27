#pragma once
#ifndef ATMOSPHERERENDERING_H
#define ATMOSPHERERENDERING_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Texture;
	class Camera;

	class TOYGE_CORE_API AtmosphereRendering : public RenderAction
	{
	public:
		AtmosphereRendering();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		//CLASS_SET(SunLight, Ptr<DirectionalLightComponent>, _sun);

		CLASS_SET(SunDirection, XMFLOAT3, _sunDirection);

		CLASS_SET(SunRadiance, XMFLOAT3, _sunRadiance);

		CLASS_SET(SunRenderRadius, float, _sunRenderRadius);

		XMFLOAT3 ComputeSunRadianceAt(const XMFLOAT3 & sunDir, const XMFLOAT3 & sunRadiance, float height);

		void RecomputeSunRenderColor();

	private:
		Ptr<RenderEffect> _fx;
		Ptr<RenderEffect> _refineFX;
		int32_t _numSampleLines;
		int32_t _maxSamplesPerLine;
		int32_t _initalSampleStep;
		//Ptr<DirectionalLightComponent> _sun;
		XMFLOAT3 _sunDirection;
		XMFLOAT3 _sunRadiance;
		Ptr<Texture> _opticalDepthLUT;
		Ptr<Texture> _inScatteringLUTR;
		Ptr<Texture> _inScatteringLUTM;
		float _earthRadius;
		float _atmosphereTopHeight;
		float _particleScaleHeightR;
		float _particleScaleHeightM;
		float3 _scatteringR;
		float3 _scatteringM;
		float3 _attenuationR;
		float3 _attenuationM;
		float _phaseG_M;
		float _sunRenderRadius;
		XMFLOAT3 _sunRenderColor;

		void InitOpticalDepthLUT();

		void InitInScatteringLUT();

		Ptr<Texture> InitSampleLines(const float2 & renderTargetSize, const Ptr<Camera> & camera, const float2 & lightPosH);

		void InitSampleCoordsTex(
			const Ptr<Texture> & sampleLinesTex,
			const Ptr<Texture> & linearDepthTex,
			Ptr<Texture> & outSampleCoordTex,
			Ptr<Texture> & outCameraDepthTex,
			Ptr<Texture> & outDepthStencilTex);

		Ptr<Texture> RefineSamples(
			const Ptr<Texture> & sampleCoordsTex,
			const Ptr<Texture> & cameraDepthTex,
			const Ptr<Camera> & camera,
			const float2 & lightPosH);

		void MarkRayMarchingSamples(const Ptr<Texture> & interpolationSourceTex, const Ptr<Texture> & depthStencilTex);

		void DoRayMarching(
			const Ptr<Texture> & sampleCoordsTex,
			const Ptr<Texture> & cameraDepthTex,
			const Ptr<Texture> & depthStencilTex,
			const Ptr<Camera> & camera,
			Ptr<Texture> & outLightAccumTex,
			Ptr<Texture> & outAttenuationTex);

		void InterpolateRestSamples(
			const Ptr<Texture> & interpolationSourceTex,
			const Ptr<Texture> & cameraDepthTex,
			const Ptr<Texture> & lightAccumTex,
			const Ptr<Texture> & attenuationTex,
			Ptr<Texture> & outLightAccumTex,
			Ptr<Texture> & outAttenuationTex);

		void UnWarpSamples(
			const Ptr<Texture> & lightAccumTex,
			const Ptr<Texture> & attenuationTex,
			const Ptr<Texture> & sceneTex,
			const ResourceView & target);

		void RenderSun(
			const float2 & lightPosH,
			const Ptr<Camera> & camera,
			const ResourceView & target,
			const Ptr<Texture> & cullDepthStencil);
	};
}

#endif