#include "ToyGE\RenderEngine\Widget.h"

namespace ToyGE
{
	Widget::Widget()
		: _pos(0.0f)
	{
		for (auto & button : _mouseDownMap)
			button = false;
	}

	float2 Widget::LocalPosToScreen(const float2 & localPos) const
	{
		auto parent = GetParent();
		if (parent)
			return parent->LocalPosToScreen(GetPos()) + localPos;
		else
			return GetPos() + localPos;
	}

	float2 Widget::ScreenPosToLocal(const float2 & screenPos) const
	{
		auto parent = GetParent();
		if (parent)
			return screenPos - parent->LocalPosToScreen(GetPos());
		else
			return screenPos - GetPos();
	}

	bool Widget::ProcessMouseIn(const float2 & mouseLocalPos)
	{
		_onMouseInEvent();

		return true;
	}

	bool Widget::ProcessMouseMove(const float2 & mouseLocalPos)
	{
		_onMouseMoveEvent(mouseLocalPos);

		return true;
	}

	bool Widget::ProcessMouseOut()
	{
		for (auto & button : _mouseDownMap)
			button = false;
		_onMouseOutEvent();

		return true;
	}

	bool Widget::ProcessMouseButtonDown(MouseButton button, const float2 & mouseLocalPos)
	{
		_mouseDownMap[button] = true;
		_onMouseButtonDownEvent(button, mouseLocalPos);
		return true;
	}

	bool Widget::ProcessMouseButtonUp(MouseButton button, const float2 & mouseLocalPos)
	{
		_onMouseButtonUpEvent(button, mouseLocalPos);

		if (_mouseDownMap[button])
		{
			_mouseDownMap[button] = false;
			_onMouseButtonClickEvent(button, mouseLocalPos);
		}
		return true;
	}
}