#pragma once
#ifndef FONTFACTORY_H
#define FONTFACTORY_H

#include "ToyGE\RenderEngine\Font\Font.h"

namespace ToyGE
{
	class FontAsset;
	class File;

	class TOYGE_CORE_API FontFactory
	{
	public:
		virtual ~FontFactory() = default;

		virtual Ptr<Font> CreateNewFont() = 0;
	};
}

#endif