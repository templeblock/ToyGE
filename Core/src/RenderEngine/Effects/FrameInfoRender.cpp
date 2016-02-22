#include "ToyGE\RenderEngine\Effects\FrameInfoRender.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\RenderEngine\Font\FontRenderer.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Camera.h"

namespace ToyGE
{
	Ptr<class Camera> FrameInfoRender::traceCamera;

	void FrameInfoRender::Render(const Ptr<class RenderTargetView> & target)
	{
		auto font = Global::GetFont();
		if (!font)
			return;

		float4 color = float4(1.0f, 1.0f, 0.0f, 1.0f);

		float y = 0.0f;
		float gap = 5.0f;

		float drawTextSize = 15.0f;
		float ascentPixels = std::ceil(font->GetAsset()->GetAscent() / (font->GetAsset()->GetAscent() - font->GetAsset()->GetDescent())) * drawTextSize;
		y += ascentPixels;

		std::stringstream ss;
		ss.setf(std::ios::fixed, std::ios::floatfield);
		ss.precision(2);

		// Adapter
		auto _adapterInfoRenderer = font->CreateRenderer();
		_adapterInfoRenderer->SetText(Global::GetRenderEngine()->GetSelectedDeviceAdapter().description);
		_adapterInfoRenderer->SetColor(color);
		_adapterInfoRenderer->Render(target, float2(0.0f, y), drawTextSize);
		y += drawTextSize + gap;

		// FPS
		auto _fpsRenderer = font->CreateRenderer();
		_fpsRenderer->SetColor(color);
		float fps = Global::GetInfo()->fps;
		ss	<< fps << " FPS"
			<< "  FrameTime: " << 1000.0f / fps << " ms";
		_fpsRenderer->SetText(ss.str());
		_fpsRenderer->Render(target, float2(0.0f, y), drawTextSize);
		y += drawTextSize + gap;

		// Camera
		if (traceCamera)
		{
			auto _cameraInfoRenderer = font->CreateRenderer();
			_cameraInfoRenderer->SetColor(color);
			ss.str("");
			auto & pos = traceCamera->GetPos();
			ss  << "CameraPos "
				<< "X:" << pos.x() << " "
				<< "Y:" << pos.y() << " "
				<< "Z:" << pos.z();
			_cameraInfoRenderer->SetText(ss.str());
			_cameraInfoRenderer->Render(target, float2(0.0f, y), drawTextSize);
		}
	}
}