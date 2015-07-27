#pragma once
#ifndef DEPTHSTENCILSHADERATE_H
#define DEPTHSTENCILSHADERATE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class TOYGE_CORE_API DepthStencilState
	{
	public:
		DepthStencilState(const DepthStencilStateDesc & desc);

		virtual ~DepthStencilState() = default;

		const DepthStencilStateDesc & Desc() const
		{
			return _desc;
		}

	private:
		DepthStencilStateDesc _desc;
	};
}

#endif