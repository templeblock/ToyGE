#pragma once
#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"
#include "boost\signals2.hpp"

namespace ToyGE
{
	enum InputDeviceType : uint32_t
	{
		INPUT_DEVICE_KEYBORD = 0UL,
		INPUT_DEVICE_MOUSE = 1UL
	};

	enum InputDeviceTypeNum : uint32_t
	{
		NUM_INPUT_DEVICE_TYPE = INPUT_DEVICE_MOUSE + 1
	};

	class TOYGE_CORE_API InputDevice
	{
	public:
		InputDevice(InputDeviceType type)
			: _type(type)
		{

		}

	protected:
		InputDeviceType _type;
	};


	enum KeyCode
	{
		KEY_Escape = 0x01,
		KEY_1 = 0x02,
		KEY_2 = 0x03,
		KEY_3 = 0x04,
		KEY_4 = 0x05,
		KEY_5 = 0x06,
		KEY_6 = 0x07,
		KEY_7 = 0x08,
		KEY_8 = 0x09,
		KEY_9 = 0x0A,
		KEY_0 = 0x0B,
		KEY_Minus = 0x0C,		// - on main keyboard
		KEY_Equals = 0x0D,
		KEY_BackSpace = 0x0E,		// backspace
		KEY_Tab = 0x0F,
		KEY_Q = 0x10,
		KEY_W = 0x11,
		KEY_E = 0x12,
		KEY_R = 0x13,
		KEY_T = 0x14,
		KEY_Y = 0x15,
		KEY_U = 0x16,
		KEY_I = 0x17,
		KEY_O = 0x18,
		KEY_P = 0x19,
		KEY_LeftBracket = 0x1A,
		KEY_RightBracket = 0x1B,
		KEY_Enter = 0x1C,		// Enter on main keyboard
		KEY_LeftCtrl = 0x1D,
		KEY_A = 0x1E,
		KEY_S = 0x1F,
		KEY_D = 0x20,
		KEY_F = 0x21,
		KEY_G = 0x22,
		KEY_H = 0x23,
		KEY_J = 0x24,
		KEY_K = 0x25,
		KEY_L = 0x26,
		KEY_Semicolon = 0x27,
		KEY_Apostrophe = 0x28,
		KEY_Grave = 0x29,		// accent grave
		KEY_LeftShift = 0x2A,
		KEY_BackSlash = 0x2B,
		KEY_Z = 0x2C,
		KEY_X = 0x2D,
		KEY_C = 0x2E,
		KEY_V = 0x2F,
		KEY_B = 0x30,
		KEY_N = 0x31,
		KEY_M = 0x32,
		KEY_Comma = 0x33,
		KEY_Period = 0x34,		// . on main keyboard
		KEY_Slash = 0x35,		// / on main keyboard
		KEY_RightShift = 0x36,
		KEY_NumPadStar = 0x37,		// * on numeric keypad
		KEY_LeftAlt = 0x38,
		KEY_Space = 0x39,
		KEY_CapsLock = 0x3A,
		KEY_F1 = 0x3B,
		KEY_F2 = 0x3C,
		KEY_F3 = 0x3D,
		KEY_F4 = 0x3E,
		KEY_F5 = 0x3F,
		KEY_F6 = 0x40,
		KEY_F7 = 0x41,
		KEY_F8 = 0x42,
		KEY_F9 = 0x43,
		KEY_F10 = 0x44,
		KEY_NumLock = 0x45,
		KEY_ScrollLock = 0x46,
		KEY_NumPad7 = 0x47,
		KEY_NumPad8 = 0x48,
		KEY_NumPad9 = 0x49,
		KEY_NumPadMinus = 0x4A,		// - on numeric keypad
		KEY_NumPad4 = 0x4B,
		KEY_NumPad5 = 0x4C,
		KEY_NumPad6 = 0x4D,
		KEY_NumPadPlus = 0x4E,		// + on numeric keypad
		KEY_NumPad1 = 0x4F,
		KEY_NumPad2 = 0x50,
		KEY_NumPad3 = 0x51,
		KEY_NumPad0 = 0x52,
		KEY_NumPadPeriod = 0x53,		// . on numeric keypad
		KEY_OEM_102 = 0x56,		// <> or \| on RT 102-key keyboard (Non-U.S.)
		KEY_F11 = 0x57,
		KEY_F12 = 0x58,
		KEY_F13 = 0x64,		//                     (NEC PC98)
		KEY_F14 = 0x65,		//                     (NEC PC98)
		KEY_F15 = 0x66,		//                     (NEC PC98)
		KEY_Kana = 0x70,		// (Japanese keyboard)
		KEY_ABNT_C1 = 0x73,		// /? on Brazilian keyboard
		KEY_Convert = 0x79,		// (Japanese keyboard)
		KEY_NoConvert = 0x7B,		// (Japanese keyboard)
		KEY_Yen = 0x7D,		// (Japanese keyboard)
		KEY_ABNT_C2 = 0x7E,		// Numpad . on Brazilian keyboard
		KEY_NumPadEquals = 0x8D,		// = on numeric keypad (NEC PC98)
		KEY_PrevTrack = 0x90,		// Previous Track (DKS_CIRCUMFLEX on Japanese keyboard)
		KEY_AT = 0x91,		//                     (NEC PC98)
		KEY_Colon = 0x92,		//                     (NEC PC98)
		KEY_Underline = 0x93,		//                     (NEC PC98)
		KEY_Kanji = 0x94,		// (Japanese keyboard)
		KEY_Stop = 0x95,		//                     (NEC PC98)
		KEY_AX = 0x96,		//                     (Japan AX)
		KEY_Unlabeled = 0x97,		//                        (J3100)
		KEY_NextTrack = 0x99,		// Next Track
		KEY_NumPadEnter = 0x9C,		// Enter on numeric keypad
		KEY_RightCtrl = 0x9D,
		KEY_Mute = 0xA0,		// Mute
		KEY_Calculator = 0xA1,		// Calculator
		KEY_PlayPause = 0xA2,		// Play / Pause
		KEY_MediaStop = 0xA4,		// Media Stop
		KEY_VolumeDown = 0xAE,		// Volume -
		KEY_VolumeUp = 0xB0,		// Volume +
		KEY_WebHome = 0xB2,		// Web home
		KEY_NumPadComma = 0xB3,		// , on numeric keypad (NEC PC98)
		KEY_NumPadSlash = 0xB5,		// / on numeric keypad
		KEY_SysRQ = 0xB7,
		KEY_RightAlt = 0xB8,		// right Alt
		KEY_Pause = 0xC5,		// Pause
		KEY_Home = 0xC7,		// Home on arrow keypad
		KEY_UpArrow = 0xC8,		// UpArrow on arrow keypad
		KEY_PageUp = 0xC9,		// PgUp on arrow keypad
		KEY_LeftArrow = 0xCB,		// LeftArrow on arrow keypad
		KEY_RightArrow = 0xCD,		// RightArrow on arrow keypad
		KEY_End = 0xCF,		// End on arrow keypad
		KEY_DownArrow = 0xD0,		// DownArrow on arrow keypad
		KEY_PageDown = 0xD1,		// PgDn on arrow keypad
		KEY_Insert = 0xD2,		// Insert on arrow keypad
		KEY_Delete = 0xD3,		// Delete on arrow keypad
		KEY_LeftWin = 0xDB,		// Left Windows key
		KEY_RightWin = 0xDC,		// Right Windows key
		KEY_Apps = 0xDD,		// AppMenu key
		KEY_Power = 0xDE,		// System Power
		KEY_Sleep = 0xDF,		// System Sleep
		KEY_Wake = 0xE3,		// System Wake
		KEY_WebSearch = 0xE5,		// Web Search
		KEY_WebFavorites = 0xE6,		// Web Favorites
		KEY_WebRefresh = 0xE7,		// Web Refresh
		KEY_WebStop = 0xE8,		// Web Stop
		KEY_WebForward = 0xE9,		// Web Forward
		KEY_WebBack = 0xEA,		// Web Back
		KEY_MyComputer = 0xEB,		// My Computer
		KEY_Mail = 0xEC,		// Mail
		KEY_MediaSelect = 0xED,		// Media Select

		KEY_Any
	};

	class TOYGE_CORE_API InputKeyboard : public InputDevice, public std::enable_shared_from_this<InputKeyboard>
	{
		typedef void(KeyEventType)(const Ptr<InputKeyboard> &, KeyCode);

	public:
		InputKeyboard()
			: InputDevice(INPUT_DEVICE_KEYBORD)
		{
			_keyState.fill(false);
		}

		bool KeyUp(KeyCode code) const
		{
			return !_keyState[code];
		}

		bool KeyDown(KeyCode code) const
		{
			return _keyState[code];
		}

		boost::signals2::signal<KeyEventType> & KeyUpEvent()
		{
			return _keyUpEvent;
		}

		boost::signals2::signal<KeyEventType> & KeyDownEvent()
		{
			return _keyDownEvent;
		}

		static bool ExistKeyDown(KeyCode code);
		static bool ExistKeyUp(KeyCode code);

	protected:
		std::array<bool, 256> _keyState; // keydown 1, keyup 0
		boost::signals2::signal<KeyEventType> _keyUpEvent;
		boost::signals2::signal<KeyEventType> _keyDownEvent;

		void KeyStateChanged(KeyCode code, bool prevState)
		{
			auto curState = _keyState[code];
			if (prevState == true && curState == false)
			{
				_keyUpEvent(shared_from_this(), code);
			}
			else if (prevState == false && curState == true)
			{
				_keyDownEvent(shared_from_this(), code);
			}
		}
	};


	enum MouseButton : uint32_t
	{
		MOUSE_BUTTON_L = 0UL,
		MOUSE_BUTTON_R = 1UL,
		MOUSE_BUTTON_M = 2UL,
		MOUSE_BUTTON_4 = 3UL,
		MOUSE_BUTTON_5 = 4UL
	};

	namespace MouseButtonNum
	{
		enum
		{
			SIZE = MOUSE_BUTTON_5 + 1UL
		};
	}

	class TOYGE_CORE_API InputMouse : public InputDevice, public std::enable_shared_from_this<InputMouse>
	{
	public:
		typedef void(ButtonChangeEventType)(const Ptr<InputMouse> &, MouseButton);
		typedef void(MouseWheelEventType)(const Ptr<InputMouse> &, int);
		typedef void(MouseMoveEventType)(const Ptr<InputMouse> &, int, int);

		InputMouse()
			: InputDevice(INPUT_DEVICE_MOUSE)
		{
			_buttonState.fill(false);
		}

		bool ButtonUp(MouseButton button)
		{
			return !_buttonState[button];
		}

		bool ButtonDown(MouseButton button)
		{
			return _buttonState[button];
		}

		virtual int2 PosScreen() const = 0;
		virtual int2 PosWindow() const = 0;

		boost::signals2::signal<ButtonChangeEventType> & OnButtonDownEvent()
		{
			return _buttonDownEvent;
		}
		boost::signals2::signal<ButtonChangeEventType> & OnButtonUpEvent()
		{
			return _buttonUpEvent;
		}
		boost::signals2::signal<MouseWheelEventType> & OnMouseWheelEvent()
		{
			return _wheelEvent;
		}
		boost::signals2::signal<MouseMoveEventType> & OnMouseMoveEvent()
		{
			return _moveEvent;
		}

	protected:
		std::array<bool, 5> _buttonState; //down 1, up 0
		boost::signals2::signal<ButtonChangeEventType> _buttonDownEvent;
		boost::signals2::signal<ButtonChangeEventType> _buttonUpEvent;
		boost::signals2::signal<MouseWheelEventType> _wheelEvent;
		boost::signals2::signal<MouseMoveEventType> _moveEvent;

		void ButtonStateChanged(MouseButton button, bool prevState)
		{
			auto curState = _buttonState[button];
			if (prevState == true && curState == false)
			{
				_buttonUpEvent(shared_from_this(), button);
			}
			else if (prevState == false && curState == true)
			{
				_buttonDownEvent(shared_from_this(), button);
			}
		}

		void MouseWheel(int wheelData)
		{
			_wheelEvent(shared_from_this(), wheelData);
		}

		void MouseMoved(int relativeX, int relativeY)
		{
			_moveEvent(shared_from_this(), relativeX, relativeY);
		}
	};
}

#endif