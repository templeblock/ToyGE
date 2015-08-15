#pragma once
#ifndef POSTPROCESSVOLUMETRICLIGHT_H
#define POSTPROCESSVOLUMETRICLIGHT_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class LightComponent;

	class TOYGE_CORE_API PostProcessVolumetricLight : public RenderAction
	{
	public:
		PostProcessVolumetricLight();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_SET(Light, Ptr<LightComponent>, _light);
		CLASS_GET(Light, Ptr<LightComponent>, _light);

	private:
		Ptr<RenderEffect> _fx;
		Ptr<LightComponent> _light;

		Ptr<Texture> Startup(const Ptr<Texture> & sceneTex, const Ptr<Texture> & linearDepthTex);
	};
}

#endif