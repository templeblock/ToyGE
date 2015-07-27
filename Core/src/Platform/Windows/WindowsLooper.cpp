#include "ToyGE\Platform\Windows\WindowsLooper.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\App.h"
#include "ToyGE\RenderEngine\RenderEngine.h"

namespace ToyGE
{
	void WindowsLooper::EnterLoop()
	{
		MSG msg;
		while (true)
		{
			if ( ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) )
			{
				if (msg.message == WM_QUIT)
				{
					break;
				}
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			else
			{
				Frame();
			}
		}
	}
}