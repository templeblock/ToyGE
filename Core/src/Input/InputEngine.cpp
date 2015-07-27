#include "ToyGE\Input\InputEngine.h"
#include "ToyGE\Platform\Window.h"

namespace ToyGE
{
	InputEngine::InputEngine()
	{
		_rawInputCallBack = std::bind(&InputEngine::OnRawInput, this, std::placeholders::_1, std::placeholders::_2);
	}

	void InputEngine::SetWindow(const Ptr<Window> & window)
	{
		/*if (_windowListener == nullptr)
			_windowListener = std::make_shared<InputEngineWindowListener>(shared_from_this());

		if (_window != nullptr)
			_window->RemoveListener(_windowListener);

		_window = window;
		if (_window != nullptr)
			_window->AddListener(_windowListener);*/
		if (_window != nullptr)
			_rawInputConnection.disconnect();
			//_window->OnRawInputEvent().disconnect(_rawInputCallBack);
		_window = window;
		if (window != nullptr)
			_window->OnRawInputEvent().connect(_rawInputCallBack);
	}
}