#pragma once
#ifndef INPUTENGINE_H
#define INPUTENGINE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

#include "ToyGE\Platform\Window.h"
#include "ToyGE\Input\InputDevice.h"

namespace ToyGE
{
	//class InputEngineWindowListener;

	class TOYGE_CORE_API InputEngine : public std::enable_shared_from_this<InputEngine>
	{
	public:
		InputEngine();

		void SetWindow(const Ptr<Window> & window);

		virtual void RefreshInputDevices() = 0;

		virtual void OnRawInput(const Ptr<Window> & window, long long handle) = 0;

		int32_t NumInputDevices(InputDeviceType type)
		{
			return static_cast<int32_t>(_inputDevices[type].size());
		}

		Ptr<InputDevice> GetInputDevice(InputDeviceType type, int32_t index) const
		{
			return _inputDevices[type][index];
		}

	protected:
		Ptr<Window> _window;
		boost::signals2::connection _rawInputConnection;
		std::function<Window::RawInputEventType> _rawInputCallBack;
		//std::shared_ptr<InputEngineWindowListener> _windowListener;
		std::array<std::vector<Ptr<InputDevice>>, InputDeviceTypeNum::NUM_INPUT_DEVICE_TYPE> _inputDevices;

	//private:
	//	void OnRawInput(const Ptr<Window> & window, long long handle);
	};

	/*class TOYGE_CORE_API InputEngineWindowListener : public WindowListener
	{
	public:
		InputEngineWindowListener(const InputEnginePtr & inputEngine)
			: _inputEngine(inputEngine)
		{

		}

		void OnRawInput(const Ptr<Window> & window, long long handle) override
		{
			if (_inputEngine)
				_inputEngine->OnRawInput(window, handle);
		}

	private:
		InputEnginePtr _inputEngine;
	};*/
}

#endif