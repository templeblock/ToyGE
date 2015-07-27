#pragma once
#ifndef WIDGETCONTAINER_H
#define WIDGETCONTAINER_H

#include "ToyGE\RenderEngine\Widget.h"

namespace ToyGE
{
	class TOYGE_CORE_API WidgetContainer : public Widget
	{
		friend class HUDRenderer;
	public:
		virtual ~WidgetContainer() = default;

		virtual void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		void AddChild(const Ptr<Widget> & child)
		{
			_children.push_back(child);
		}

		int32_t NumChildren() const
		{
			return static_cast<int32_t>(_children.size());
		}

		const Ptr<Widget> & GetChild(int32_t index)
		{
			return _children[index];
		}

	protected:
		std::vector<Ptr<Widget>> _children;
		float2 _prevMouseLocalPos;

		virtual void RenderSelf(const Ptr<RenderSharedEnviroment> & sharedEnviroment){};

		virtual bool ProcessMouseIn(const float2 & mouseLocalPos) override;

		virtual bool ProcessMouseMove(const float2 & mouseLocalPos) override;

		virtual bool ProcessMouseOut() override;

		virtual bool ProcessMouseButtonDown(MouseButton button, const float2 & mouseLocalPos) override;

		virtual bool ProcessMouseButtonUp(MouseButton button, const float2 & mouseLocalPos) override;
	};
}

#endif