#pragma once
#ifndef TONEMAPPING_H
#define TONEMAPPING_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	DECLARE_SHADER(, BrightPassPS, SHADER_PS, "BrightPass", "BrightPassPS", SM_4);
	DECLARE_SHADER(, ToneMappingLUTPS, SHADER_PS, "ToneMappingLUT", "ToneMappingLUTPS", SM_4);
	DECLARE_SHADER(, ToneMappingPS, SHADER_PS, "ToneMapping", "ToneMappingPS", SM_4);

	class TOYGE_CORE_API ToneMapping : public RenderAction
	{
	public:
		ToneMapping();

		virtual void Render(const Ptr<RenderView> & view) override;

		CLASS_GET(BrightPassThreshold, float, _brightPassThreshold);
		CLASS_SET(BrightPassThreshold, float, _brightPassThreshold);

	private:
		float _brightPassThreshold;
		float _brightPassScaleParam;

		PooledTextureRef BrightPass(const Ptr<Texture> & sceneTex, const Ptr<Texture> & adaptedExposureScale);

		PooledTextureRef BloomDownSample(const Ptr<Texture> & inTex);

		void BloomBlur(const Ptr<Texture> & inTex, float blurRadius);

		void BloomUpSample(const float & weight, const Ptr<Texture> & lowResTex, const Ptr<Texture> & highResTex);

		PooledTextureRef ComputeToneMappingLUT();

		void DoToneMapping(
			const Ptr<Texture> & scene,
			const Ptr<Texture> & bloomTex,
			const Ptr<Texture> & adaptedExposureScale,
			const Ptr<Texture> & lut,
			const Ptr<RenderTargetView> & target);
	};
}

#endif