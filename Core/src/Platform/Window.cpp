#include "ToyGE\Platform\Window.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\App.h"

namespace ToyGE
{
	Window::Window(const WindowCreateParams & params)
		: _name(params.name), _width(params.width), _height(params.height), _x(params.x), _y(params.y)
	{

	}

	void Window::Resize(int32_t width, int32_t height)
	{
		int32_t prevWidth = _width;
		int32_t prevHeight = _height;
		_width = width;
		_height = height;

		_resizeEvent(shared_from_this(), prevWidth, prevHeight);
	}

	void Window::PaintCallback()
	{
		_paintEvent(shared_from_this());
	}

	void Window::DestroyCallBack()
	{
		Global::GetApp()->Destroy();

		_destroyEvent(shared_from_this());
	}

	void Window::RawInputCallBack(int64_t handle)
	{
		_rawInputEvent(shared_from_this(), handle);
	}
}