#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

#include <boost\signals2.hpp>

namespace ToyGE
{
	struct WindowCreateParams
	{
		String name;
		String title;
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
	};

	class TOYGE_CORE_API Window : public std::enable_shared_from_this<Window>
	{
	public:
		typedef void(PaintEventType)(const Ptr<Window> &);
		typedef void(ResizeEventType)(const Ptr<Window> &, int32_t, int32_t);
		typedef void(DestroyEventType)(const Ptr<Window> &);
		typedef void(RawInputEventType)(const Ptr<Window> &, long long);

		Window(const WindowCreateParams & params);

		virtual ~Window() = default;

		virtual void Init() = 0;

		int32_t Width() const
		{
			return _width;
		}
		int32_t Height() const
		{
			return _height;
		}
		int32_t X() const
		{
			return _x;
		}
		int32_t Y() const
		{
			return _y;
		}
		float AspectRatio() const
		{
			return static_cast<float>(Width()) / static_cast<float>(Height());
		}

		void Resize(int32_t width, int32_t height);

		boost::signals2::signal<PaintEventType> & OnPaintEvent()
		{
			return _paintEvent;
		}

		boost::signals2::signal<ResizeEventType> & OnResizeEvent()
		{
			return _resizeEvent;
		}

		boost::signals2::signal<DestroyEventType> & OnDestroyEvent()
		{
			return _destroyEvent;
		}

		boost::signals2::signal<RawInputEventType> & OnRawInputEvent()
		{
			return _rawInputEvent;
		}

		virtual HWINDOW WindowHandle() const = 0;

		virtual void SetTitle(const String & title)
		{
			_title = title;
		}

		virtual void Show() = 0;

	protected:
		String _name;
		String _title;
		int32_t _x;
		int32_t _y;
		int32_t _width;
		int32_t _height;

		//std::vector<std::shared_ptr<WindowListener>> _listenerList;
		boost::signals2::signal<PaintEventType> _paintEvent;
		boost::signals2::signal<ResizeEventType> _resizeEvent;
		boost::signals2::signal<DestroyEventType> _destroyEvent;
		boost::signals2::signal<RawInputEventType> _rawInputEvent;

		void PaintCallback();
		void DestroyCallBack();
		void RawInputCallBack(int64_t handle);
	};
}

#endif