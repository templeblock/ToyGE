#include "ToyGE\Platform\Windows\WindowsPlatformFactory.h"
#include "ToyGE\Platform\Windows\WindowsWindow.h"
#include "ToyGE\Platform\Windows\WindowsLooper.h"
#include "ToyGE\Platform\Windows\WindowsDllLoader.h"

namespace ToyGE
{
	Ptr<Window> WindowsPlatformFactory::CreateRenderWindow(const WindowCreateParams & params)
	{
		auto window = std::make_shared<WindowsWindow>(params);
		if (window)
			return window;
		else
			return Ptr<Window>();
	}

	Ptr<Looper> WindowsPlatformFactory::CreateLooper()
	{
		return std::make_shared<WindowsLooper>();
	}

	void WindowsPlatformFactory::ShowMessage(const String & msg)
	{
		::MessageBoxA(0, msg.c_str(), 0, 0);
	}
}