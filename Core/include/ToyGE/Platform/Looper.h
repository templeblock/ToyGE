#pragma once
#ifndef LOOPER_H
#define LOOPER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Kernel\Timer.h"
#include "boost\signals2.hpp"

namespace ToyGE
{
	class TOYGE_CORE_API Looper
	{
	public:
		typedef void(FrameEventType)();

		Looper();

		virtual ~Looper() = default;

		virtual void EnterLoop() = 0;

		boost::signals2::signal<FrameEventType> & FrameEvent()
		{
			return _frameEvent;
		}

	protected:
		void Frame();

	private:
		Timer _timer;
		boost::signals2::signal<FrameEventType> _frameEvent;
	};
}

#endif