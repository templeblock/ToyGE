#pragma once
#ifndef GLOBALINFO_H
#define GLOBALINFO_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class GlobalInfo
	{
	public:
		GlobalInfo()
			: _fps(0.0f),
			_elapsedTime(0.0f),
			_frameCount(-1)
		{

		}

		CLASS_GET(FPS, float, _fps);
		CLASS_SET(FPS, float, _fps);

		CLASS_GET(ElapsedTime, float, _elapsedTime);
		CLASS_SET(ElapsedTime, float, _elapsedTime);

		CLASS_GET(FrameCount, int32_t, _frameCount);
		CLASS_SET(FrameCount, int32_t, _frameCount);

	private:
		float _fps;
		float _elapsedTime;
		int32_t _frameCount;
	};
}

#endif