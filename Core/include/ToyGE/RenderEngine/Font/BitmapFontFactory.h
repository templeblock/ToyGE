#pragma once
#ifndef BITMAPFONTFACOTRY_H
#define BITMAPFONTFACOTRY_H

#include "ToyGE\RenderEngine\Font\FontFactory.h"

namespace ToyGE
{
	class TOYGE_CORE_API BitmapFontFactory : public FontFactory
	{
	public:
		virtual ~BitmapFontFactory() = default;

		Ptr<Font> LoadFont(const Ptr<File> & file) override;
	};
}

#endif