#pragma once
#ifndef FONTRENDERER_H
#define FONTRENDERER_H

#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class TOYGE_CORE_API FontRenderer
	{
	public:
		virtual ~FontRenderer() = default;

		virtual Ptr<Font> GetFont() const = 0;

		virtual void SetText(const String & text) = 0;

		virtual void SetColor(const float4 & color) = 0;

		virtual void Render(const Ptr<class RenderTargetView> & target, const float2 & pos, float height, float width = 0.0f) = 0;
	};
}

#endif