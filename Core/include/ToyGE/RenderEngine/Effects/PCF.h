#pragma once
#ifndef PCF_H
#define PCF_H

#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	class PCF : public ShadowRenderTechnique
	{
	public:
		PCF();

		virtual ~PCF() = default;

		Ptr<Texture> ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light) override;

		virtual void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		virtual void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		CLASS_SET(FilterSize, float, _filterSize);
		CLASS_GET(FilterSize, float, _filterSize);

	protected:
		float _filterSize;
	};
}

#endif
