#pragma once
#ifndef RENDERVIEWPORT_H
#define RENDERVIEWPORT_H

namespace ToyGE
{
	class TOYGE_CORE_API RenderViewport
	{
	public:
		float topLeftX = FLT_MAX;
		float topLeftY = FLT_MAX;
		float width = FLT_MAX;
		float height = FLT_MAX;
		float minDepth = FLT_MAX;
		float maxDepth = FLT_MAX;

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