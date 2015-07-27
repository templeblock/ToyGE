#pragma once
#ifndef WINDOWSINPUTDEVICE_H
#define WINDOWSINPUTDEVICE_H

#include "ToyGE\Input\InputDevice.h"

namespace ToyGE
{
	class WindowsInputDevice
	{
	public:
		WindowsInputDevice(HANDLE handle)
			: _handle(handle)
		{

		}

		HANDLE GetHandle() const
		{
			return _handle;
		}

	protected:
		HANDLE _handle;
	};

	class WindowsInputKeyboard : public InputKeyboard, public WindowsInputDevice
	{
	public:
		WindowsInputKeyboard(HANDLE handle)
			: WindowsInputDevice(handle)
		{

		}

		void OnRawInput(const RAWKEYBOARD & keyboard);
	};

	class WindowsInputMouse : public InputMouse, public WindowsInputDevice
	{
	public:
		WindowsInputMouse(HANDLE handle)
			: WindowsInputDevice(handle)
		{

		}

		void OnRawInput(const RAWMOUSE & mouse);

		int2 PosScreen() const override;
		int2 PosWindow() const override;
	};
}

#endif