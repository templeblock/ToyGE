#include "ToyGE\RenderEngine\WidgetContainer.h"

namespace ToyGE
{
	void WidgetContainer::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		RenderSelf(sharedEnviroment);

		for (auto & child : _children)
			child->Render(sharedEnviroment);
	}

	bool WidgetContainer::ProcessMouseIn(const float2 & mouseLocalPos)
	{
		for (auto & child : _children)
		{
			if (child->Intersect(mouseLocalPos))
				child->ProcessMouseIn(mouseLocalPos - child->GetPos());
		}
		_prevMouseLocalPos = mouseLocalPos;

		return Widget::ProcessMouseIn(mouseLocalPos);
	}

	bool WidgetContainer::ProcessMouseMove(const float2 & mouseLocalPos)
	{
		for (auto & child : _children)
		{
			bool prevMouseIn = child->Intersect(_prevMouseLocalPos);
			bool curMouseIn = child->Intersect(mouseLocalPos);
			if (curMouseIn)
			{
				if (!prevMouseIn)
					child->ProcessMouseIn(mouseLocalPos - child->GetPos());
				else
					child->ProcessMouseMove(mouseLocalPos - child->GetPos());
			}
			else
			{
				if (prevMouseIn)
					child->ProcessMouseOut();
			}
		}
		_prevMouseLocalPos = mouseLocalPos;

		return Widget::ProcessMouseMove(mouseLocalPos);
	}

	bool WidgetContainer::ProcessMouseOut()
	{
		for (auto & child : _children)
		{
			if (child->Intersect(_prevMouseLocalPos))
				child->ProcessMouseOut();
		}

		return Widget::ProcessMouseOut();
	}

	bool WidgetContainer::ProcessMouseButtonDown(MouseButton button, const float2 & mouseLocalPos)
	{
		for (auto & child : _children)
		{
			if (child->Intersect(mouseLocalPos))
			{
				if (child->ProcessMouseButtonDown(button, mouseLocalPos - child->GetPos()))
					return true;
			}
		}

		return Widget::ProcessMouseButtonDown(button, mouseLocalPos);
	}

	bool WidgetContainer::ProcessMouseButtonUp(MouseButton button, const float2 & mouseLocalPos)
	{
		for (auto & child : _children)
		{
			if (child->Intersect(mouseLocalPos))
			{
				if (child->ProcessMouseButtonUp(button, mouseLocalPos - child->GetPos()))
					return true;
			}
		}

		return Widget::ProcessMouseButtonUp(button, mouseLocalPos);
	}
}