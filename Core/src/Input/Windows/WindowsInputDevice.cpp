#include "ToyGE\Input\Windows\WindowsInputDevice.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\Platform\Window.h"

namespace ToyGE
{
	namespace
	{
		static int32_t const VK_MAPPING[] =
		{
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			KEY_BackSpace,
			KEY_Tab,
			-1,
			-1,
			-1,
			KEY_Enter,
			-1,
			-1,
			KEY_LeftShift,
			KEY_LeftCtrl,
			KEY_LeftAlt,
			KEY_Pause,
			KEY_CapsLock,
			KEY_Kana,
			-1,
			-1,
			-1,
			-1,
			-1,
			KEY_Escape,
			KEY_Convert,
			KEY_NoConvert,
			-1,
			-1,
			KEY_Space,
			KEY_PageUp,
			KEY_PageDown,
			KEY_End,
			KEY_Home,
			KEY_LeftArrow,
			KEY_UpArrow,
			KEY_RightArrow,
			KEY_DownArrow,
			-1,
			-1,
			-1,
			-1,
			KEY_Insert,
			KEY_Delete,
			-1,
			KEY_0,
			KEY_1,
			KEY_2,
			KEY_3,
			KEY_4,
			KEY_5,
			KEY_6,
			KEY_7,
			KEY_8,
			KEY_9,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			KEY_A,
			KEY_B,
			KEY_C,
			KEY_D,
			KEY_E,
			KEY_F,
			KEY_G,
			KEY_H,
			KEY_I,
			KEY_J,
			KEY_K,
			KEY_L,
			KEY_M,
			KEY_N,
			KEY_O,
			KEY_P,
			KEY_Q,
			KEY_R,
			KEY_S,
			KEY_T,
			KEY_U,
			KEY_V,
			KEY_W,
			KEY_X,
			KEY_Y,
			KEY_Z,
			KEY_LeftWin,
			KEY_RightWin,
			KEY_Apps,
			-1,
			KEY_Sleep,
			KEY_NumPad0,
			KEY_NumPad1,
			KEY_NumPad2,
			KEY_NumPad3,
			KEY_NumPad4,
			KEY_NumPad5,
			KEY_NumPad6,
			KEY_NumPad7,
			KEY_NumPad8,
			KEY_NumPad9,
			KEY_NumPadStar,
			KEY_Equals,
			KEY_BackSlash,
			KEY_Minus,
			-1,
			KEY_Slash,
			KEY_F1,
			KEY_F2,
			KEY_F3,
			KEY_F4,
			KEY_F5,
			KEY_F6,
			KEY_F7,
			KEY_F8,
			KEY_F9,
			KEY_F10,
			KEY_F11,
			KEY_F12,
			KEY_F13,
			KEY_F14,
			KEY_F15,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			KEY_NumLock,
			KEY_ScrollLock,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			KEY_LeftShift,
			KEY_RightShift,
			KEY_LeftCtrl,
			KEY_RightCtrl,
			KEY_LeftAlt,
			KEY_RightAlt,
			KEY_WebBack,
			KEY_WebForward,
			KEY_WebRefresh,
			KEY_WebStop,
			KEY_WebSearch,
			KEY_WebFavorites,
			KEY_WebHome,
			KEY_Mute,
			KEY_VolumeDown,
			KEY_VolumeUp,
			KEY_NextTrack,
			KEY_PrevTrack,
			KEY_MediaStop,
			KEY_PlayPause,
			KEY_Mail,
			KEY_MediaSelect,
			-1,
			-1,
			-1,
			-1,
			KEY_Comma,
			KEY_NumPadPlus,
			-1,
			KEY_NumPadMinus,
			KEY_NumPadPeriod,
			KEY_Slash,
			KEY_Grave,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			KEY_LeftBracket,
			KEY_BackSlash,
			KEY_RightBracket,
			KEY_Grave,
			-1,
			-1,
			-1,
			KEY_OEM_102,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1
		};
	}

	void WindowsInputKeyboard::OnRawInput(const RAWKEYBOARD & keyboard)
	{
		auto keyCode = VK_MAPPING[keyboard.VKey];
		if (keyCode > 0)
		{
			auto prevState = _keyState[keyCode];
			_keyState[keyCode] = ((keyboard.Flags & 1UL) == RI_KEY_MAKE);
			KeyStateChanged(static_cast<KeyCode>(keyCode), prevState);
		}
	}

	void WindowsInputMouse::OnRawInput(const RAWMOUSE & mouse)
	{
		auto prevButtonState = _buttonState;
		for (int i = 0; i < 5; ++i)
		{
			if (mouse.usButtonFlags & (1UL << (i * 2)))
				_buttonState[i] = true;
			if (mouse.usButtonFlags & (1UL << (i * 2 + 1)))
				_buttonState[i] = false;
		}

		for (int i = 0; i < 5; ++i)
		{
			if (prevButtonState[i] != _buttonState[i])
				ButtonStateChanged(static_cast<MouseButton>(i), prevButtonState[i]);
		}

		if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
			MouseWheel(mouse.usButtonData);

		if (MOUSE_MOVE_RELATIVE == mouse.usFlags)
		{
			if (mouse.lLastX != 0 || mouse.lLastY != 0)
				MouseMoved(mouse.lLastX, mouse.lLastY);
		}
	}

	int2 WindowsInputMouse::PosScreen() const
	{
		POINT pos;
		::GetCursorPos(&pos);
		return int2(pos.x, pos.y);
	}

	int2 WindowsInputMouse::PosWindow() const
	{
		POINT pos;
		::GetCursorPos(&pos);
		::ScreenToClient(Global::GetRenderEngine()->GetWindow()->WindowHandle(), &pos);
		return int2(pos.x, pos.y);
	}
}