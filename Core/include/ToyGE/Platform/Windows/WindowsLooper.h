#pragma once
#ifndef WINDOWSLOOPER_H
#define WINDOWSLOOPER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Platform\Looper.h"

namespace ToyGE
{
	class WindowsLooper : public Looper
	{
	public:
		void EnterLoop() override;
	};
}

#endif