#pragma once
#ifndef SSR_H
#define SSR_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	DECLARE_SHADER(, HZBBuildPS, SHADER_PS, "SSR", "HZBBuildPS", SM_4);
	DECLARE_SHADER(, ScreenSpaceReflectionPS, SHADER_PS, "SSR", "ScreenSpaceReflectionPS", SM_4);

	class TOYGE_CORE_API SSR : public RenderAction
	{
	public:
		SSR();

		virtual void Render(const Ptr<RenderView> & view) override;

		CLASS_SET(SSRMaxRoughness, float, _ssrMaxRoughness)
		CLASS_GET(SSRMaxRoughness, float, _ssrMaxRoughness)

		CLASS_SET(SSRIntensity, float, _ssrIntensity)
		CLASS_GET(SSRIntensity, float, _ssrIntensity)

	private:
		float _ssrMaxRoughness;
		float _ssrIntensity;

		PooledTextureRef BuildHZB(const Ptr<Texture> & depthTex);
	};
}

#endif