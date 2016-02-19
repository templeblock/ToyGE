#pragma once
#ifndef BITMAPFONTFACOTRY_H
#define BITMAPFONTFACOTRY_H

#include "ToyGE\RenderEngine\Font\FontFactory.h"
#include "ToyGE\RenderEngine\Font\BitmapFont.h"

namespace ToyGE
{
	class TOYGE_CORE_API BitmapFontFactory : public FontFactory
	{
	public:
		virtual ~BitmapFontFactory() = default;

		virtual Ptr<Font> CreateNewFont()
		{
			return std::make_shared<BitmapFont>();
		}
	};
}

#endif