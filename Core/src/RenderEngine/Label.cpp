#include "ToyGE\RenderEngine\Label.h"
#include "ToyGE\RenderEngine\Font\FontRenderer.h"
#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"

namespace ToyGE
{
	Label::Label()
		: _textRenderSize(12.0f),
		_textColor(1.0f)
	{

	}

	void Label::SetFont(const Ptr<Font> & font)
	{
		if (!_fontRenderer || font != _fontRenderer->GetFont())
			_fontRenderer = font->CreateRenderer();
	}

	Ptr<Font> Label::GetFont() const
	{
		return _fontRenderer ? _fontRenderer->GetFont() : nullptr;
	}

	void Label::RenderSelf(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		Panel::RenderSelf(sharedEnviroment);

		if (!_fontRenderer)
			return;

		float2 penPos = LocalPosToScreen(0.0f);
		auto font = _fontRenderer->GetFont();
		penPos.y += font->GetAscent() / (font->GetAscent() - font->GetDescent()) * _textRenderSize.y;

		_fontRenderer->SetText(_text);
		_fontRenderer->SetColor(_textColor);
		_fontRenderer->Render(sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView(), penPos, _textRenderSize);
	}
}