#include "ToyGE\Platform\Windows\WindowsWindow.h"
#include "ToyGE\Kernel\Util.h"
#include "AntTweakBar.h"

namespace ToyGE
{
	LRESULT CALLBACK WindowsWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		WindowsWindow *pWnd = reinterpret_cast<WindowsWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (nullptr == pWnd)
		{
			return ::DefWindowProc(hWnd, msg, wParam, lParam);
		}
		else
		{
			return pWnd->MsgProc(hWnd, msg, wParam, lParam);
		}
	}

	WindowsWindow::WindowsWindow(const WindowCreateParams & params)
		: Window(params)
	{
		
	}

	void WindowsWindow::Init()
	{
		//WString wName;
		//ConvertStr_AToW(_name, wName);

		HINSTANCE hInst = ::GetModuleHandle(nullptr);

		//Register window class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = 0;
		wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = _name.c_str();
		wcex.hIconSm = 0;

		::RegisterClassEx(&wcex);

		RECT rect = { 0, 0, _width, _height };
		::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

		//Create window
		_hwnd = ::CreateWindowW(_name.c_str(), _name.c_str(), WS_OVERLAPPEDWINDOW,
			_x, _y,
			rect.right - rect.left, rect.bottom - rect.top,
			nullptr, nullptr, hInst, nullptr);

		if (!_hwnd)
		{
			return;
		}

		::GetClientRect(_hwnd, &rect);
		_x = _x;
		_y = _y;
		_width = rect.right - rect.left;
		_height = rect.bottom - rect.top;

		//Set window identifier
		::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		ShowWindow(_hwnd, SW_SHOWNORMAL);
		UpdateWindow(_hwnd);
	}

	void WindowsWindow::SetTitle(const WString & tile)
	{
		SetWindowTextW(WindowHandle(), tile.c_str());
	}

	LRESULT CALLBACK WindowsWindow::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		TwEventWin(hWnd, msg, wParam, lParam);

		switch (msg)
		{
		case WM_SIZE:
		{
			uint32_t w = LOWORD(lParam);
			uint32_t h = HIWORD(lParam);

			if (w && h && (w != Width() || h != Height()))
				Resize(w, h);
			break;
		}

		case WM_INPUT:
			RawInputCallBack(lParam);
			break;

		case WM_PAINT:
			PaintCallback();
			break;

		case WM_DESTROY:
			DestroyCallBack();
			::PostQuitMessage(0);
			break;
		}

		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}
}