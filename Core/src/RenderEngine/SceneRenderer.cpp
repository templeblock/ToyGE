#include "ToyGE\RenderEngine\SceneRenderer.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\Effects\AtmosphereRendering.h"

namespace ToyGE
{
	SceneRenderer::SceneRenderer()
	{
		_atmosphereRendering = std::make_shared<AtmosphereRendering>();
	}
}