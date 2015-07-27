#pragma once
#ifndef RASTERIZERSHADERATE_H
#define RASTERIZERSHADERATE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class TOYGE_CORE_API RasterizerState
	{
	public:
		RasterizerState(const RasterizerStateDesc & desc);

		virtual ~RasterizerState() = default;

		const RasterizerStateDesc & Desc() const
		{
			return _desc;
		}

	private:
		RasterizerStateDesc _desc;
	};
}

#endif