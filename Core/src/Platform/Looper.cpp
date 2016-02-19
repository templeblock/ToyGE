#include "ToyGE\Platform\Looper.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"

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

		// Update Info
		//auto globalInfo = Global::GetInfo();

		// Calculate FPS
		static float _elapsedTimeAccum = 0.0f;
		static int32_t _frameAccum = 0;

		++_frameAccum;
		_elapsedTimeAccum += elapsedTime;
		if (_elapsedTimeAccum >= 1.0f)
		{
			Global::GetInfo()->fps = (static_cast<float>(_frameAccum) / _elapsedTimeAccum);
			_elapsedTimeAccum = _elapsedTimeAccum - std::floor(_elapsedTimeAccum);
			_frameAccum = 0;
		}

		Global::GetInfo()->elapsedTime = elapsedTime;
		++Global::GetInfo()->frameCount;

		// Update App
		Global::GetApp()->Update(elapsedTime);

		// Render Frame
		Global::GetRenderEngine()->Render();

		RenderResource::ClearExpiredResources();

		// Tick resource pool
		TexturePool::Instance().Tick();
		BufferPool::Instance().Tick();

		// Frame Event
		_frameEvent();
	}
}
