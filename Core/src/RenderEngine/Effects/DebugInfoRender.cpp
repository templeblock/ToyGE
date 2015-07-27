#include "ToyGE\RenderEngine\Effects\DebugInfoRender.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\RenderEngine\Font\FontRenderer.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Camera.h"

namespace ToyGE
{
	DebugInfoRender::DebugInfoRender()
	{
		float4 color = float4(1.0f, 1.0f, 0.0f, 1.0f);

		WString fontName = L"STZHONGS.TTF";

		_adapterInfoRenderer = Global::GetResourceManager(RESOURCE_FONT)->As<FontManager>()->AcquireResource(fontName)->CreateRenderer();
		_adapterInfoRenderer->SetText(Global::GetRenderEngine()->GetSelectedDeviceAdapter().description);
		_adapterInfoRenderer->SetColor(color);

		_fpsRenderer = Global::GetResourceManager(RESOURCE_FONT)->As<FontManager>()->AcquireResource(fontName)->CreateRenderer();
		_fpsRenderer->SetColor(color);

		_dpRenderer = Global::GetResourceManager(RESOURCE_FONT)->As<FontManager>()->AcquireResource(fontName)->CreateRenderer();
		_dpRenderer->SetColor(color);

		_cameraInfoRenderer = Global::GetResourceManager(RESOURCE_FONT)->As<FontManager>()->AcquireResource(fontName)->CreateRenderer();
		_cameraInfoRenderer->SetColor(color);
	}

	void DebugInfoRender::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		float y = 0.0f;
		float gap = 5.0f;

		float2 drawTextSize = 14.0f;
		auto font = _adapterInfoRenderer->GetFont();
		float ascentPixels = std::ceil(font->GetAscent() / (font->GetAscent() - font->GetDescent())) * drawTextSize.y;
		y += ascentPixels;

		std::wstringstream wss;
		wss.setf(std::ios::fixed, std::ios::floatfield);
		wss.precision(2);

		//Adapter
		_adapterInfoRenderer->Render(sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView(), float2(0.0f, y), drawTextSize);
		y += drawTextSize.y + gap;

		//FPS
		float fps = Global::GetInfo()->GetFPS();
		wss
			<< fps << L" FPS"
			<< L"  FrameTime: " << 1000.0f / fps << L" ms";
		_fpsRenderer->SetText(wss.str());
		_fpsRenderer->Render(sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView(), float2(0.0f, y), drawTextSize);
		y += drawTextSize.y + gap;

		//Camera
		wss.str(L"");
		auto & pos = sharedEnviroment->GetView()->GetCamera()->Pos();
		wss << L"CameraPos "
			<< L"X:" << pos.x << L" "
			<< L"Y:" << pos.y << L" "
			<< L"Z:" << pos.z;
		_cameraInfoRenderer->SetText(wss.str());
		_cameraInfoRenderer->Render(sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView(), float2(0.0f, y), drawTextSize);
	}
}