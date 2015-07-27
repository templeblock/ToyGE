#pragma once
#ifndef HIZSSR_H
#define HIZSSR_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class Camera;

	class TOYGE_CORE_API HizSSR : public RenderAction
	{
	public:
		HizSSR();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _ssrFX;

		Ptr<Texture> HiZPass(const Ptr<Texture> & rawDepthTex);

		Ptr<Texture> PreIntegration(const Ptr<Texture> & hizTex, const Ptr<Camera> & camera);

		Ptr<Texture> Convolution(const Ptr<Texture> & sceneTex);

		void Tracing(
			const Ptr<Texture> & gbuffer0,
			const Ptr<Texture> & gbuffer1,
			const Ptr<Texture> & hizTex,
			const Ptr<Texture> & integrationTex,
			const Ptr<Texture> & convolTex,
			const Ptr<Camera> & camera,
			const ResourceView & target);
	};
}

#endif