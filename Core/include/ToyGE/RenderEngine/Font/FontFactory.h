#pragma once
#ifndef FONTFACTORY_H
#define FONTFACTORY_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class Font;
	class File;

	class TOYGE_CORE_API FontFactory
	{
	public:
		virtual ~FontFactory() = default;

		virtual Ptr<Font> LoadFont(const Ptr<File> & file) = 0;
	};
}

#endif