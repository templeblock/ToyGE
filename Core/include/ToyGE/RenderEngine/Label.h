#pragma once
#ifndef LABEL_H
#define LABEL_H

#include "ToyGE\RenderEngine\Panel.h"

namespace ToyGE
{
	class Font;
	class FontRenderer;

	class TOYGE_CORE_API Label : public Panel
	{
	public:
		Label();

		void SetFont(const Ptr<Font> & font);

		Ptr<Font> GetFont() const;

		CLASS_GET(Text, WString, _text);
		CLASS_SET(Text, WString, _text);

		CLASS_GET(TextRenderSize, float2, _textRenderSize);
		CLASS_SET(TextRenderSize, float2, _textRenderSize);

		CLASS_GET(TextColor, float4, _textColor);
		CLASS_SET(TextColor, float4, _textColor);

	protected:
		Ptr<FontRenderer> _fontRenderer;
		WString _text;
		float2 _textRenderSize;
		float4 _textColor;

		void RenderSelf(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;
	};
}

#endif