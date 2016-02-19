#pragma once
#ifndef RENDERVIEWPORT_H
#define RENDERVIEWPORT_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class RenderViewport
	{
	public:
		float topLeftX	= 0.0f;
		float topLeftY	= 0.0f;
		float width		= 0.0f;
		float height	= 0.0f;
		float minDepth	= 0.0f;
		float maxDepth	= 1.0f;

		bool operator==(const RenderViewport & rhs)
		{
			return topLeftX == rhs.topLeftX
				&& topLeftY == rhs.topLeftY
				&& width == rhs.width
				&& height == rhs.height
				&& minDepth == rhs.minDepth
				&& maxDepth == rhs.maxDepth;
		}

		bool operator!=(const RenderViewport & rhs)
		{
			return !(this->operator==(rhs));
		}
	};
}

#endif