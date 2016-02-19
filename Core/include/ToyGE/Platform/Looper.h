#pragma once
#ifndef LOOPER_H
#define LOOPER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
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

		CLASS_SET(Exit, bool, _bExit);

	protected:
		Timer _timer;
		bool _bExit = false;
		boost::signals2::signal<FrameEventType> _frameEvent;

		void Frame();
	};
}

#endif