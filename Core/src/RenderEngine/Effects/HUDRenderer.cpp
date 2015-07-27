#include "ToyGE\RenderEngine\Effects\HUDRenderer.h"
#include "ToyGE\RenderEngine\WidgetContainer.h"
#include "ToyGE\Kernel\Global.h"

namespace ToyGE
{
	HUDRenderer::HUDRenderer()
	{
		_root = std::make_shared<WidgetContainer>();

		auto numMouses = Global::GetInputEngine()->NumInputDevices(INPUT_DEVICE_MOUSE);
		for (int32_t i = 0; i < numMouses; ++i)
		{
			auto mouse = std::static_pointer_cast<InputMouse>(Global::GetInputEngine()->GetInputDevice(INPUT_DEVICE_MOUSE, i));
			mouse->OnButtonDownEvent().connect(std::bind(&HUDRenderer::OnMouseButtonDown, this, std::placeholders::_1, std::placeholders::_2));
			mouse->OnButtonUpEvent().connect(std::bind(&HUDRenderer::OnMouseButtonUp, this, std::placeholders::_1, std::placeholders::_2));
			mouse->OnMouseMoveEvent().connect(std::bind(&HUDRenderer::OnMouseMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}

	}

	void HUDRenderer::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		if (!_root)
			return;

		_root->Render(sharedEnviroment);
	}

	void HUDRenderer::OnMouseMove(const Ptr<InputMouse> & mouse, int relativeX, int relativeY)
	{
		auto posWindow = mouse->PosWindow();
		float2 pos = float2(static_cast<float>(posWindow.x), static_cast<float>(posWindow.y));
		_root->ProcessMouseMove(pos);
		_curPos = pos;
	}

	void HUDRenderer::OnMouseButtonDown(const Ptr<InputMouse> &, MouseButton button)
	{
		_root->ProcessMouseButtonDown(button, _curPos);
	}

	void HUDRenderer::OnMouseButtonUp(const Ptr<InputMouse> &, MouseButton button)
	{
		_root->ProcessMouseButtonUp(button, _curPos);
	}
}