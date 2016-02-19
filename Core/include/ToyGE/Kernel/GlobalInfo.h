#pragma once
#ifndef GLOBALINFO_H
#define GLOBALINFO_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class GlobalInfo
	{
	public:
		float	fps = 0.0f;
		float	elapsedTime = 0.0f;
		int32_t frameCount = 0;
	};
}

#endif