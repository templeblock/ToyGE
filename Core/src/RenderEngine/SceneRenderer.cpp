#include "ToyGE\RenderEngine\SceneRenderer.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\Effects\Effects.h"
#include "ToyGE\RenderEngine\TranslucentRendering.h"

namespace ToyGE
{
	SceneRenderer::SceneRenderer()
	{
		_atmosphereRendering = std::make_shared<AtmosphereRendering>();
		_lpv = std::make_shared<LPV>();
		_volumetricLight = std::make_shared<VolumetricLight>();
		_ssrRenderer = std::make_shared<SSR>();
		_translucentRendering = std::make_shared<TranslucentRendering>();
		_envReflectionRenderer = std::make_shared<EnvironmentReflectionRenderer>();
		_eyeAdaption = std::make_shared<EyeAdaption>();
	}
}