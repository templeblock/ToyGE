#include "ToyGE\Platform\Looper.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\Kernel\App.h"
#include "ToyGE\RenderEngine\RenderEngine.h"

namespace ToyGE
{
	Looper::Looper()
	{
		_timer.Init();
	}

	void Looper::Frame()
	{
		float elapsedTime = 0.0f;
		elapsedTime = _timer.ElapsedSecondsTime();

		//Update Info
		auto globalInfo = Global::GetInfo();

		//Calculate FPS
		static float _elapsedTimeAccum = 0.0f;
		static int32_t _frameAccum = 0;

		++_frameAccum;
		_elapsedTimeAccum += elapsedTime;
		if (_elapsedTimeAccum >= 1.0f)
		{
			globalInfo->SetFPS(static_cast<float>(_frameAccum) / _elapsedTimeAccum);
			_elapsedTimeAccum = _elapsedTimeAccum - std::floor(_elapsedTimeAccum);
			_frameAccum = 0;
		}

		globalInfo->SetElapsedTime(elapsedTime);
		globalInfo->SetFrameCount(globalInfo->GetFrameCount() + 1);

		//Update App
		Global::GetApp()->Update(elapsedTime);

		//Render Frame
		Global::GetRenderEngine()->RenderFrame();

		//Frame Event
		_frameEvent();
	}
}