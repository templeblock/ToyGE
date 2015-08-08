#pragma once
#ifndef SUBSURFACESCATTERING_H
#define SUBSURFACESCATTERING_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	class LightComponent;
	class Camera;

	class TOYGE_CORE_API SubsurfaceSacttering : public RenderAction
	{
	public:
		SubsurfaceSacttering();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _fx;

		Ptr<Texture> ComputeDiffuse(
			const Ptr<Texture> & rawDepthTex,
			const Ptr<Texture> & gbuffer0,
			const Ptr<Texture> & gbuffer1,
			const Ptr<Texture> & lighting0);

		void Blur();

		void Transmittance(
			const std::vector<Ptr<LightComponent>> & lights,
			const Ptr<Camera> & camera,
			const Ptr<Texture> & rawDepthTex,
			const Ptr<Texture> & targetTex);
	};
}

#endif