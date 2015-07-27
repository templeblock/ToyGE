#pragma once
#ifndef FONTRENDERER_H
#define FONTRENDERER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class Font;

	class TOYGE_CORE_API FontRenderer
	{
	public:
		virtual ~FontRenderer() = default;

		virtual Ptr<Font> GetFont() const = 0;

		virtual void SetText(const WString & text) = 0;

		virtual void SetColor(const float4 & color) = 0;

		virtual void Render(const ResourceView & target, const float2 & pos, const float2 pixelSize) = 0;
	};
}

#endif