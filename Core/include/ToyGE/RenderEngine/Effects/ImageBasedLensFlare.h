#pragma once
#ifndef IMAGEBASEDLENSFLARE_H
#define IMAGEBASEDLENSFLARE_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	class TOYGE_CORE_API ImageBasedLensFlare : public RenderAction
	{
	public:
		ImageBasedLensFlare();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_SET(BrightPassThreshold, float, _brightPassThreshold)
		CLASS_GET(BrightPassThreshold, float, _brightPassThreshold)

		CLASS_SET(SpriteThreshold, float, _spriteThreshold)
		CLASS_GET(SpriteThreshold, float, _spriteThreshold)

		CLASS_SET(FlareIntensity, float, _flareIntensity)
		CLASS_GET(FlareIntensity, float, _flareIntensity)

	private:
		Ptr<RenderEffect> _fx;
		Ptr<Texture> _lensTex;
		float _brightPassThreshold;
		float _spriteThreshold;
		float _flareIntensity;

		Ptr<Texture> BrightPass(const Ptr<Texture> & sceneTex);

		Ptr<Texture> Setup(const Ptr<Texture> & brightPassTex);

		void LensBlur(const Ptr<Texture> & setupTex, const Ptr<Texture> & target);
	};
}

#endif