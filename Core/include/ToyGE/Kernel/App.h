#pragma once
#ifndef APP_H
#define APP_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class TOYGE_CORE_API App
	{
	public:
		virtual ~App() = default;

		virtual void Init(){}

		virtual void Update(float elapsedTime){}

		virtual void Destroy(){}
	};
}

#endif