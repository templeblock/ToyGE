#pragma once
#ifndef SIGNEDDISTANCEFIELDFONTFACTORY_H
#define SIGNEDDISTANCEFIELDFONTFACTORY_H

#include "ToyGE\RenderEngine\Font\FontFactory.h"
#include "ToyGE\RenderEngine\Font\SignedDistanceFieldFont.h"

namespace ToyGE
{
	class TOYGE_CORE_API SignedDistanceFieldFontFactory : public FontFactory
	{
	public:
		virtual ~SignedDistanceFieldFontFactory() = default;

		virtual Ptr<Font> CreateNewFont() override
		{
			return std::make_shared<SignedDistanceFieldFont>();
		}
	};
}

#endif