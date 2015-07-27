#include "ToyGE\Input\InputDevice.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Input\InputEngine.h"

namespace ToyGE
{
	bool InputKeyboard::ExistKeyDown(KeyCode code)
	{
		auto numKeyboards = Global::GetInputEngine()->NumInputDevices(INPUT_DEVICE_KEYBORD);
		bool bKeyDown = false;
		for (int32_t i = 0; i < numKeyboards; ++i)
		{
			auto keyboard = std::static_pointer_cast<InputKeyboard>(Global::GetInputEngine()->GetInputDevice(INPUT_DEVICE_KEYBORD, i));
			if (keyboard->KeyDown(code))
				bKeyDown = true;
		}
		return bKeyDown;
	}

	bool InputKeyboard::ExistKeyUp(KeyCode code)
	{
		auto numKeyboards = Global::GetInputEngine()->NumInputDevices(INPUT_DEVICE_KEYBORD);
		bool bKeyUp = false;
		for (int32_t i = 0; i < numKeyboards; ++i)
		{
			auto keyboard = std::static_pointer_cast<InputKeyboard>(Global::GetInputEngine()->GetInputDevice(INPUT_DEVICE_KEYBORD, i));
			if (keyboard->KeyUp(code))
				bKeyUp = true;
		}
		return bKeyUp;
	}
}