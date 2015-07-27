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
		
		globalInfo->SetElapsedTime(elapsedTime);
		globalInfo->SetFPS(1.0f / elapsedTime);
		globalInfo->SetFrameCount(globalInfo->GetFrameCount() + 1);

		//Update App
		Global::GetApp()->Update(elapsedTime);

		//Render Frame
		Global::GetRenderEngine()->RenderFrame();

		//Frame Event
		_frameEvent();
	}

	//bool Looper::CalculateFrame(float timeElapsed)
	//{
	//	++_frameCount;
	//	_timeAccum += timeDelta;
	//	if (_timeAccum >= 1000.0f)
	//	{
	//		_fps = (static_cast<float>(_frameCount) * 1000.0f / _timeAccum);
	//		_frameCount = 0;
	//		_timeAccum = 0.0f;
	//		return true;
	//	}
	//	return false;
	//}
}