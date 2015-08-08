#pragma once
#ifndef WINDOWSWINDOW_H
#define WINDOWSWINDOW_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Platform\Window.h"

namespace ToyGE
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowCreateParams & params);

		void Init() override;

		HWINDOW WindowHandle() const override
		{
			return _hwnd;
		}

		void SetTitle(const WString & tile) override;

		void Show() override;

	private:
		HWND _hwnd;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	};
}

#endif