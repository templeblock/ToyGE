#pragma once
#ifndef IMAGEBASEDLENSFLARE_H
#define IMAGEBASEDLENSFLARE_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	DECLARE_SHADER(, LensBlurBrightPassPS, SHADER_PS, "ImageBasedLensFlare", "BrightPassPS", SM_4);
	DECLARE_SHADER(, LensFlareSetupPS, SHADER_PS, "ImageBasedLensFlare", "LensFlareSetupPS", SM_4);
	DECLARE_SHADER(, ExtractSpritePointsPS, SHADER_PS, "ImageBasedLensFlare", "ExtractSpritePointsPS", SM_4);
	DECLARE_SHADER(, LensBlurVS, SHADER_VS, "ImageBasedLensFlare", "LensBlurVS", SM_4);
	DECLARE_SHADER(, LensBlurGS, SHADER_GS, "ImageBasedLensFlare", "LensBlurGS", SM_4);
	DECLARE_SHADER(, LensBlurPS, SHADER_PS, "ImageBasedLensFlare", "LensBlurPS", SM_4);

	class TOYGE_CORE_API ImageBasedLensFlare : public RenderAction
	{
	public:
		ImageBasedLensFlare();

		void Render(const Ptr<RenderView> & view) override;

		CLASS_SET(BrightPassThreshold, float, _brightPassThreshold)
		CLASS_GET(BrightPassThreshold, float, _brightPassThreshold)

		CLASS_SET(SpriteThreshold, float, _spriteThreshold)
		CLASS_GET(SpriteThreshold, float, _spriteThreshold)

		CLASS_SET(FlareIntensity, float, _flareIntensity)
		CLASS_GET(FlareIntensity, float, _flareIntensity)

	private:
		Ptr<RenderBuffer> _indirectAgsBuf;
		Ptr<Texture> _lensTex;
		float _brightPassThreshold;
		float _spriteThreshold;
		float _flareIntensity;

		PooledTextureRef BrightPass(const Ptr<Texture> & sceneTex);

		PooledTextureRef Setup(const Ptr<Texture> & brightPassTex);

		void LensBlur(const Ptr<Texture> & setupTex, const Ptr<Texture> & target);
	};
}

#endif