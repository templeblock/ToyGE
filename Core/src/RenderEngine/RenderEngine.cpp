#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderFramework.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\TransientBuffer.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

namespace ToyGE
{
	RenderEngine::RenderEngine(const Ptr<Window> & window)
		: _window(window)
	{

	}

	RenderEngine::~RenderEngine()
	{
		TwTerminate();
	}

	void RenderEngine::Startup()
	{
		DoStartup();

		int32_t initNumTextChars = 1024;
		int32_t charVertexSize = static_cast<int32_t>(sizeof(float) * (3 + 3));
		_transientBufferMap[TRANSIENTBUFFER_TEXT_VERTEX] = GetRenderFactory()->CreateTransientBuffer(charVertexSize, initNumTextChars * 4, BUFFER_BIND_VERTEX);
		_transientBufferMap[TRANSIENTBUFFER_TEXT_VERTEX]->Register();
		_transientBufferMap[TRANSIENTBUFFER_TEXT_INDEX] = GetRenderFactory()->CreateTransientBuffer(sizeof(uint32_t), initNumTextChars * 6, BUFFER_BIND_INDEX);
		_transientBufferMap[TRANSIENTBUFFER_TEXT_INDEX]->Register();
	}

	void RenderEngine::RenderFrame()
	{
		if (_renderFramework)
			_renderFramework->Render();
		SwapChain();
	}

	void RenderEngine::PresentToBackBuffer(const ResourceView & resource)
	{
		Transform(resource, _defaultRenderTarget->CreateTextureView());
	}
}

