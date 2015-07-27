#include "ToyGE\D3D11\D3D11RenderEngineCreateDef.h"
#include "ToyGE\Kernel\Global.h"

void CreateRenderEngine(const std::shared_ptr<ToyGE::Window> & window, ToyGE::RenderEngine **ppRenderEngine)
{
	using ToyGE::D3D11RenderEngine;
	*ppRenderEngine = new D3D11RenderEngine(window);
}