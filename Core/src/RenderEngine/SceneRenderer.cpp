#include "ToyGE\RenderEngine\SceneRenderer.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\Effects\AtmosphereRendering.h"

namespace ToyGE
{
	SceneRenderer::SceneRenderer()
	{
		_atmosphereRendering = std::make_shared<AtmosphereRendering>();
	}

	void SceneRenderer::SetSunDirection(const XMFLOAT3 & sunDirection)
	{
		auto xmDir = XMLoadFloat3(&sunDirection);
		xmDir = XMVector3Normalize(xmDir);
		XMStoreFloat3(&_sunDirection, xmDir);
		_atmosphereRendering->SetSunDirection(_sunDirection);
		_atmosphereRendering->RecomputeSunRenderColor();
	}

	void SceneRenderer::SetSunRadiance(const XMFLOAT3 & sunRadiance)
	{
		_sunRadiance = sunRadiance;
		_atmosphereRendering->SetSunRadiance(sunRadiance);
		_atmosphereRendering->RecomputeSunRenderColor();
	}

	void SceneRenderer::SetSunRenderRadius(float sunRenderRadius)
	{
		_sunRenderRadius = sunRenderRadius;
		_atmosphereRendering->SetSunRenderRadius(sunRenderRadius);
	}

	void SceneRenderer::SetSunLight(const Ptr<class DirectionalLightComponent> & light)
	{
		_sunLight = light;
	}

	void SceneRenderer::UpdateSunLight()
	{
		if (_sunLight)
		{
			auto radiance = _atmosphereRendering->ComputeSunRadianceAt(_sunDirection, XMFLOAT3(_sunRadiance.x, _sunRadiance.y, _sunRadiance.z), 1.0f);
			auto maxV = std::max<float>(std::max<float>(radiance.x, radiance.y), radiance.z);
			if (maxV > 1e-4f)
			{
				float3 color = float3(radiance.x, radiance.y, radiance.z) / maxV;
				_sunLight->SetColor(color);
				_sunLight->SetIntensity(maxV);
			}
			else
			{
				_sunLight->SetColor(0.0f);
				_sunLight->SetIntensity(0.0f);
			}
			_sunLight->SetDirection(_sunDirection);
		}
	}
}