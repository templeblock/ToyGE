#pragma once
#ifndef WIDGET_H
#define WIDGET_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\Input\InputDevice.h"
#include "boost\signals2.hpp"

namespace ToyGE
{
#define DECLARE_EVENT(eventName, param) \
	protected: boost::signals2::signal<On##eventName##EventType> param;\
	public:\
	boost::signals2::signal<On##eventName##EventType> & On##eventName##Event()\
{\
	return param;\
}

	class TOYGE_CORE_API Widget : public RenderAction
	{
		friend class HUDRenderer;
		friend class WidgetContainer;
	public:
		typedef void(OnMouseInEventType)();
		typedef void(OnMouseOutEventType)();
		typedef void(OnMouseMoveEventType)(float2 mouseLocalPos);
		typedef void(OnMouseButtonDownEventType)(MouseButton button, const float2 & mouseLocalPos);
		typedef void(OnMouseButtonUpEventType)(MouseButton button, const float2 & mouseLocalPos);
		typedef void(OnMouseButtonClickEventType)(MouseButton button, const float2 & mouseLocalPos);

		Widget();

		virtual ~Widget() = default;

		//virtual float4 GetBoundRect() const = 0;

		virtual bool Intersect(const float2 & localPos) const
		{
			return false;
		};

		DECLARE_EVENT(MouseIn, _onMouseInEvent);
		DECLARE_EVENT(MouseOut, _onMouseOutEvent);
		DECLARE_EVENT(MouseMove, _onMouseMoveEvent);
		DECLARE_EVENT(MouseButtonDown, _onMouseButtonDownEvent);
		DECLARE_EVENT(MouseButtonUp, _onMouseButtonUpEvent);
		DECLARE_EVENT(MouseButtonClick, _onMouseButtonClickEvent);

		CLASS_GET(Pos, float2, _pos);
		CLASS_SET(Pos, float2, _pos);

		CLASS_GET(Parent, Ptr<Widget>, _parent);
		CLASS_SET(Parent, Ptr<Widget>, _parent);

		float2 LocalPosToScreen(const float2 & localPos) const;

		float2 ScreenPosToLocal(const float2 & screenPos) const;

	protected:
		float2 _pos;
		Ptr<Widget> _parent;
		std::array<bool, MouseButtonNum::SIZE> _mouseDownMap;

		virtual bool ProcessMouseIn(const float2 & mouseLocalPos);

		virtual bool ProcessMouseMove(const float2 & mouseLocalPos);

		virtual bool ProcessMouseOut();

		virtual bool ProcessMouseButtonDown(MouseButton button, const float2 & mouseLocalPos);

		virtual bool ProcessMouseButtonUp(MouseButton button, const float2 & mouseLocalPos);
	};
}

#endif