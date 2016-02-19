#pragma once
#ifndef TIMER_H
#define TIMER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class Timer
	{
	public:
		Timer()
			: _prevTick(0)
		{
			long long frequency;
#ifdef TOYGE_PLATFORM_WINDOWS
			::QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
#endif
			_secondsPerTick = 1.0f / frequency;
		}

		void Init()
		{
			_prevTick = CurTick();
		}

		float ElapsedSecondsTime()
		{
			long long curTick = CurTick();
			float ret = (curTick - _prevTick) * _secondsPerTick;
			_prevTick = curTick;
			return ret;
		}

		long long CurTick() const
		{
			long long curTick;
#ifdef TOYGE_PLATFORM_WINDOWS
			::QueryPerformanceCounter((LARGE_INTEGER*)&curTick);
#endif
			return curTick;
		}

	private:
		float _secondsPerTick;
		long long _prevTick;
	};
}

#endif