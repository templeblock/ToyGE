#pragma once
#ifndef HDR_H
#define HDR_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	DECLARE_SHADER(, ComputeIlluminaceInitialPS, SHADER_PS, "HDR", "ComputeIlluminaceInitialPS", SM_4);
	DECLARE_SHADER(, ComputeIlluminaceReducePS, SHADER_PS, "HDR", "ComputeIlluminaceReducePS", SM_4);
	DECLARE_SHADER(, ComputeAvgAndAdaptedIlumPS, SHADER_PS, "HDR", "ComputeAvgAndAdaptedIlumPS", SM_4);
	DECLARE_SHADER(, BrightPassPS, SHADER_PS, "HDR", "BrightPassPS", SM_4);
	DECLARE_SHADER(, HDRCombinePS, SHADER_PS, "HDR", "HDRCombinePS", SM_4);

	class TOYGE_CORE_API HDR : public RenderAction
	{
	public:
		HDR();

		virtual void Render(const Ptr<RenderView> & view) override;

		CLASS_GET(BrightPassThreshold, float, _brightPassThreshold);
		CLASS_SET(BrightPassThreshold, float, _brightPassThreshold);

		CLASS_GET(BrightPassScaleParam, float, _brightPassScaleParam);
		CLASS_SET(BrightPassScaleParam, float, _brightPassScaleParam);

	private:
		PooledTextureRef _prevAvgAdaptedIlumTexRef;
		PooledTextureRef _avgAdaptedIlumTexRef;
		float _brightPassThreshold;
		float _brightPassScaleParam;

		PooledTextureRef SceneDownSample(const Ptr<Texture> & scene);

		void ComputeIlluminace(const Ptr<Texture> & downSampleTex);

		PooledTextureRef BrightPass(const Ptr<Texture> & downSampleTex);

		PooledTextureRef BloomDownSample(const Ptr<Texture> & inTex);

		void BloomUpSample(const Ptr<Texture> & lowResTex, const Ptr<Texture> & highResTex);

		void Combine(
			const Ptr<Texture> & scene,
			const Ptr<Texture> & blurTex,
			const Ptr<Texture> & streakTex,
			const Ptr<RenderTargetView> & target);
	};
}

#endif