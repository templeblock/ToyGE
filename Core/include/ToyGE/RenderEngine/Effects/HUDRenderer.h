#pragma once
#ifndef HUDRENDERER_H
#define HUDRENDERER_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\Input\InputEngine.h"

namespace ToyGE
{
	class WidgetContainer;

	class TOYGE_CORE_API HUDRenderer : public RenderAction
	{
	public:
		HUDRenderer();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_GET(Root, Ptr<WidgetContainer>, _root);
		CLASS_SET(Root, Ptr<WidgetContainer>, _root);

	private:
		Ptr<WidgetContainer> _root;
		float2 _curPos;

		void OnMouseMove(const Ptr<InputMouse> &, int relativeX, int relativeY);

		void OnMouseButtonDown(const Ptr<InputMouse> &, MouseButton button);

		void OnMouseButtonUp(const Ptr<InputMouse> &, MouseButton button);
	};
}

#endif