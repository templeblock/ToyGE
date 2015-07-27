#pragma once
#ifndef BITMAPFONTRENDERER_H
#define BITMAPFONTRENDERER_H

#include "ToyGE\RenderEngine\Font\FontRenderer.h"
#include "ToyGE\RenderEngine\Font\BitmapFont.h"
#include "ToyGE\RenderEngine\TransientBuffer.h"

namespace ToyGE
{
	class RenderInput;

	class TOYGE_CORE_API BitmapFontRenderer : public FontRenderer
	{
	public:
		BitmapFontRenderer(const Ptr<BitmapFont> & font);

		virtual ~BitmapFontRenderer() = default;

		Ptr<Font> GetFont() const override
		{
			return _font;
		}

		void SetText(const WString & text) override;

		void SetColor(const float4 & color) override
		{
			_color = color;
		}

		void Render(const ResourceView & target, const float2 & screenPos, const float2 pixelSize) override;

	private:
		Ptr<BitmapFont> _font;
		WString _text;
		float4 _color;
		Ptr<RenderInput> _textRenderInput;
		Ptr<SubAlloc> _textRenderVB;
		Ptr<SubAlloc> _textRenderIB;
		//Ptr<RenderBuffer> _textRenderVB;
		//Ptr<RenderBuffer> _textRenderIB;

		void UpdateRenderBuffers();
	};
}

#endif