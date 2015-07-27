#include "ToyGE\Input\Windows\WindowsInputEngine.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\Input\Windows\WindowsInputDevice.h"
#include <hidusage.h>

namespace ToyGE
{
	void WindowsInputEngine::RefreshInputDevices()
	{
		if (_window == nullptr)
			return;
		for (auto & i : _inputDevices)
			i.clear();

		uint32_t numDevice;
		if (::GetRawInputDeviceList(nullptr, &numDevice, sizeof(RAWINPUTDEVICELIST)) != 0)
		{
			Logger::LogLine("error> GetRawInputDeviceList fail");
			return;
		}

		std::vector<RAWINPUTDEVICELIST> inputDevicesList(numDevice);
		::GetRawInputDeviceList(&inputDevicesList[0], &numDevice, sizeof(RAWINPUTDEVICELIST));

		std::vector<RAWINPUTDEVICE> registerInputDevice;
		for (auto & deviceList : inputDevicesList)
		{
			RAWINPUTDEVICE device;
			device.usUsagePage = HID_USAGE_PAGE_GENERIC;
			device.hwndTarget = _window->WindowHandle();

			bool bDeviceSupport = true;
			switch (deviceList.dwType)
			{
			case RIM_TYPEKEYBOARD:
				device.dwFlags = 0;
				device.usUsage = HID_USAGE_GENERIC_KEYBOARD;
				_inputDevices[INPUT_DEVICE_KEYBORD].push_back(std::make_shared<WindowsInputKeyboard>(deviceList.hDevice));
				break;

			case RIM_TYPEMOUSE:
				device.dwFlags = 0;
				device.usUsage = HID_USAGE_GENERIC_MOUSE;
				_inputDevices[INPUT_DEVICE_MOUSE].push_back(std::make_shared<WindowsInputMouse>(deviceList.hDevice));
				break;

			default:
				bDeviceSupport = false;
				break;
			}

			if (bDeviceSupport)
				registerInputDevice.push_back(device);
		}

		if (::RegisterRawInputDevices(&registerInputDevice[0], static_cast<UINT>(registerInputDevice.size()), sizeof(RAWINPUTDEVICE)) == false)
		{
			Logger::LogLine("error> RegisterRawInputDevices fail");
			return;
		}

	}

	void WindowsInputEngine::OnRawInput(const Ptr<Window> & window, long long handle)
	{
		if (window != _window)
			return;

		uint32_t dataSize;
		if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(handle), RID_INPUT, nullptr, &dataSize, sizeof(RAWINPUTHEADER)) != 0)
		{
			Logger::LogLine("error> GetRawInputData fail");
			return;
		}
		std::vector<uint8_t> inputData(dataSize);
		::GetRawInputData(reinterpret_cast<HRAWINPUT>(handle), RID_INPUT, &inputData[0], &dataSize, sizeof(RAWINPUTHEADER));

		RAWINPUT *pInputData = reinterpret_cast<RAWINPUT*>(&inputData[0]);
		switch (pInputData->header.dwType)
		{
		case RIM_TYPEKEYBOARD:
			for (auto & device : _inputDevices[INPUT_DEVICE_KEYBORD])
			{
				auto keyboard = std::static_pointer_cast<WindowsInputKeyboard>(device);
				if (keyboard->GetHandle() == pInputData->header.hDevice)
				{
					keyboard->OnRawInput(pInputData->data.keyboard);
				}
			}
			break;

		case RIM_TYPEMOUSE:
			for (auto & device : _inputDevices[INPUT_DEVICE_MOUSE])
			{
				auto keyboard = std::static_pointer_cast<WindowsInputMouse>(device);
				if (keyboard->GetHandle() == pInputData->header.hDevice)
				{
					keyboard->OnRawInput(pInputData->data.mouse);
				}
			}
			break;

		default:
			break;
		}
	}
}