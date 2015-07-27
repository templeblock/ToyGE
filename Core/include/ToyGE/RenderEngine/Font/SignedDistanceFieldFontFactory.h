#pragma once
#ifndef SIGNEDDISTANCEFIELDFONTFACTORY_H
#define SIGNEDDISTANCEFIELDFONTFACTORY_H

#include "ToyGE\RenderEngine\Font\FontFactory.h"

namespace ToyGE
{
	class TOYGE_CORE_API SignedDistanceFieldFontFactory : public FontFactory
	{
	public:
		virtual ~SignedDistanceFieldFontFactory() = default;

		Ptr<Font> LoadFont(const Ptr<File> & file) override;
	};
}

#endif