#pragma once
#ifndef SSR_H
#define SSR_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	class TOYGE_CORE_API SSR : public RenderAction
	{
	public:
		SSR();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_SET(SSRMaxRoughness, float, _ssrMaxRoughness)
		CLASS_GET(SSRMaxRoughness, float, _ssrMaxRoughness)

		CLASS_SET(SSRIntensity, float, _ssrIntensity)
		CLASS_GET(SSRIntensity, float, _ssrIntensity)

	private:
		Ptr<RenderEffect> _fx;
		float _ssrMaxRoughness;
		float _ssrIntensity;

		//void InitDither();

		Ptr<Texture> BuildHZB(const Ptr<Texture> & depthTex);
	};
}

#endif