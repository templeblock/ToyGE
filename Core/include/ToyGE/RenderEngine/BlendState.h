#pragma once
#ifndef BLENDSTATE_H
#define BLENDSTATE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class TOYGE_CORE_API BlendState
	{
	public:
		BlendState(const BlendStateDesc & desc);

		virtual ~BlendState() = default;

		const BlendStateDesc & Desc() const
		{
			return _desc;
		}

	private:
		BlendStateDesc _desc;
	};
}

#endif