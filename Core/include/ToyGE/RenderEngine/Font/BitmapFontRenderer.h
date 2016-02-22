#pragma once
#ifndef BITMAPFONTRENDERER_H
#define BITMAPFONTRENDERER_H

#include "ToyGE\RenderEngine\Font\FontRenderer.h"
#include "ToyGE\RenderEngine\Font\BitmapFont.h"
#include "ToyGE\RenderEngine\TransientBuffer.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	class RenderInput;

	DECLARE_SHADER(, FontRenderingVS, SHADER_VS, "FontRenderingVS", "FontRenderingVS", SM_4);
	DECLARE_SHADER(, BitmapFontRenderingPS, SHADER_PS, "FontRenderingPS", "BitmapFontRenderingPS", SM_4);

	class TOYGE_CORE_API BitmapFontRenderer : public FontRenderer
	{
	public:
		BitmapFontRenderer(const Ptr<BitmapFont> & font);

		virtual ~BitmapFontRenderer() = default;

		Ptr<Font> GetFont() const override
		{
			return _font;
		}

		void SetText(const String & text) override;

		void SetColor(const float4 & color) override
		{
			_color = color;
		}

		virtual void Render(const Ptr<RenderTargetView> & target, const float2 & screenPos, float height, float width = 0.0f) override;

	protected:
		Ptr<BitmapFont> _font;
		String			_text;
		int32_t			_textNumCharacters;
		float4			_color;

		Ptr<SubAlloc>			_vbTransientSubAlloc;
		Ptr<class VertexBuffer> _vb;
		Ptr<SubAlloc>			_ibTransientSubAlloc;
		Ptr<RenderBuffer>		_ib;

		bool _bNeedUpdate = false;

		void UpdateRenderBuffers();

		float4x4 ComputeTransform(const Ptr<RenderTargetView> & target, const float2 & screenPos, float height, float width);
	};
}

#endif