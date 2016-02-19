#pragma once
#ifndef SIGNEDDISTANCEFONTRENDERER_H
#define SIGNEDDISTANCEFONTRENDERER_H

#include "ToyGE\RenderEngine\Font\BitmapFontRenderer.h"
#include "ToyGE\RenderEngine\Font\SignedDistanceFieldFont.h"

namespace ToyGE
{
	DECLARE_SHADER(, SignedDistanceFontRenderingPS, SHADER_PS, "FontRenderingPS", "SignedDistanceFontRenderingPS", SM_4);

	class TOYGE_CORE_API SignedDistanceFontRenderer : public BitmapFontRenderer
	{
	public:
		SignedDistanceFontRenderer(const Ptr<SignedDistanceFieldFont> & font);

		virtual void Render(const Ptr<RenderTargetView> & target, const float2 & screenPos, float height, float width = 0.0f) override;
	};
}

#endif
