#pragma once
#ifndef ATMOSPHERERENDERING_H
#define ATMOSPHERERENDERING_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	DECLARE_SHADER(, InitOpticalDepthLUTPS, SHADER_PS, "AtmosphereRenderingPrecompute", "InitOpticalDepthLUTPS", SM_4);
	DECLARE_SHADER(, PreComputeSingleScatteringPS, SHADER_PS, "AtmosphereRenderingPrecompute", "PreComputeSingleScatteringPS", SM_4);
	DECLARE_SHADER(, ComputeOutRadiancePS, SHADER_PS, "AtmosphereRenderingPrecompute", "ComputeOutRadiancePS", SM_4);
	DECLARE_SHADER(, PreComputeScatteringOrderPS, SHADER_PS, "AtmosphereRenderingPrecompute", "PreComputeScatteringOrderPS", SM_4);
	DECLARE_SHADER(, AccumScatteringPS, SHADER_PS, "AtmosphereRenderingPrecompute", "AccumScatteringPS", SM_4);
	DECLARE_SHADER(, AccumSingleScatteringPS, SHADER_PS, "AtmosphereRenderingPrecompute", "AccumSingleScatteringPS", SM_4);

	DECLARE_SHADER(, InitSampleLinesPS, SHADER_PS, "AtmosphereRendering", "InitSampleLinesPS", SM_4);
	DECLARE_SHADER(, InitSampleCoordsPS, SHADER_PS, "AtmosphereRendering", "InitSampleCoordsPS", SM_4);
	DECLARE_SHADER(, RefineSamplesCS, SHADER_CS, "AtmosphereRefineSamples", "RefineSamplesCS", SM_5);
	DECLARE_SHADER(, MarkRayMarchingSamplesPS, SHADER_PS, "AtmosphereRendering", "MarkRayMarchingSamplesPS", SM_4);
	DECLARE_SHADER(, RayMarchingPS, SHADER_PS, "AtmosphereRendering", "RayMarchingPS", SM_4);
	DECLARE_SHADER(, InterpolateRestSamplesPS, SHADER_PS, "AtmosphereRendering", "InterpolateRestSamplesPS", SM_4);
	DECLARE_SHADER(, UnWarpSamplesPS, SHADER_PS, "AtmosphereRendering", "UnWarpSamplesPS", SM_4);
	DECLARE_SHADER(, AccumRayMarchingPS, SHADER_PS, "AtmosphereRendering", "AccumRayMarchingPS", SM_4);

	DECLARE_SHADER(, RenderSunVS, SHADER_VS, "AtmosphereRenderingRenderSun", "RenderSunVS", SM_4);
	DECLARE_SHADER(, RenderSunPS, SHADER_PS, "AtmosphereRenderingRenderSun", "RenderSunPS", SM_4);

	DECLARE_SHADER(, ComputeSunRadiancePS, SHADER_PS, "AtmosphereRenderingComputeSunRadiance", "ComputeSunRadiancePS", SM_4);

	DECLARE_SHADER(, RenderHemiPanoramicMapPS, SHADER_PS, "AtmosphereRendering", "RenderHemiPanoramicMapPS", SM_4);

	class Texture;
	class Camera;

	class TOYGE_CORE_API AtmosphereRendering : public RenderAction
	{
	public:
		bool bEpipolarSampling = false;

		AtmosphereRendering();

		virtual void Render(const Ptr<RenderView> & view) override;

		void RenderSun(const Ptr<RenderView> & view);

		void RenderCubeMap(const Ptr<Texture> & target);

		void RenderHemiPanoramicMap(const Ptr<Texture> & target);

		void SetSunDirection(const float3 & sunDirection)
		{
			_sunDirection = normalize(sunDirection);
		}
		void SetSunRadiance(const float3 & sunRadiance)
		{
			_sunRadiance = sunRadiance;
		}
		void SetSunRenderRadius(float sunRenderRadius)
		{
			_sunRenderRadius = sunRenderRadius;
		}

		void UpdateSunLight(const Ptr<class DirectionalLightComponent> & light);

		void UpdateAmbientAndReflectionMap(const Ptr<class Scene> & scene);

		float3 ComputeSunRadianceAt(const float3 & sunDir, const float3 & sunRadiance, float height);

		void RecomputeSunRenderColor();

	private:
		int32_t _numSampleLines;
		int32_t _maxSamplesPerLine;
		int32_t _initalSampleStep;
		int4 _lutSize;

		float _earthRadius;
		float _atmosphereTopHeight;
		float _particleScaleHeightR;
		float _particleScaleHeightM;
		float3 _scatteringR;
		float3 _scatteringM;
		float3 _attenuationR;
		float3 _attenuationM;
		float _phaseG_M;

		Ptr<Texture> _opticalDepthLUT;
		Ptr<Texture> _inScatteringLUTR;
		Ptr<Texture> _inScatteringLUTM;

		float3 _sunDirection;
		float3 _sunRadiance;
		float	 _sunRenderRadius;
		float3 _sunRenderColor;

		void InitOpticalDepthLUT();

		void InitInScatteringLUT();

		PooledTextureRef InitSampleLines(
			const float4 & viewSize,
			const float2 & lightClipPos);

		void InitSampleCoordsTex(
			const float4 & viewSize,
			const Ptr<Texture> & sampleLinesTex,
			const Ptr<Texture> & sceneLinearDepthTex,
			PooledTextureRef & outSampleCoordTex,
			PooledTextureRef & outSampleDepthTex,
			PooledTextureRef & outSampleMaskDS);

		PooledTextureRef RefineSamples(
			const float4 & viewSize,
			float depthBreakThreshold,
			const Ptr<Texture> & sampleCoordsTex,
			const Ptr<Texture> & sampleDepthTex);

		void MarkRayMarchingSamples(
			const Ptr<Texture> & interpolationSourceTex, 
			const Ptr<Texture> & sampleMaskDS);

		void DoRayMarching(
			const Ptr<RenderView> & view,
			const Ptr<Texture> & sampleCoordsTex,
			const Ptr<Texture> & depthTex,
			const Ptr<Texture> & sampleMaskDS,
			PooledTextureRef & outLightAccumTex,
			PooledTextureRef & outAttenuationTex);

		void InterpolateRestSamples(
			const float4 & viewSize,
			const Ptr<Texture> & interpolationSourceTex,
			const Ptr<Texture> & sampleDepthTex,
			const Ptr<Texture> & lightAccumTex,
			const Ptr<Texture> & attenuationTex,
			const Ptr<Texture> & sampleMaskDS,
			PooledTextureRef & outLightAccumTex,
			PooledTextureRef & outAttenuationTex);

		void UnWarpSamples(
			const float2 & lightClipPos,
			const Ptr<Texture> & lightAccumTex,
			const Ptr<Texture> & attenuationTex,
			const Ptr<Texture> & sampleLinesTex,
			const Ptr<Texture> & sampleDepthTex,
			const Ptr<Texture> & sceneTex,
			const Ptr<Texture> & sceneLinearDepthTex,
			const Ptr<RenderTargetView> & target);

		void AccumRayMarching(
			const Ptr<Texture> & lightAccumTex,
			const Ptr<Texture> & attenuationTex,
			const Ptr<RenderTargetView> & target);

		void DoRenderSun(
			const float2 & lightClipPos,
			const Ptr<RenderView> & view,
			const Ptr<RenderTargetView> & target,
			const Ptr<DepthStencilView> & sceneDepthTex);
	};
}

#endif