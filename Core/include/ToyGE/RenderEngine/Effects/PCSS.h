#pragma once
#ifndef PCSS_H
#define PCSS_H

#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\Effects\VSM.h"

namespace ToyGE
{
	class ShadowRenderTechniquePCSS_SAVSM : public ShadowRenderTechniqueSAVSM
	{
	public:
		ShadowRenderTechniquePCSS_SAVSM();

		//virtual void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		virtual void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		ShadowRenderTechniqueType Type() const override
		{
			return SHADOW_RENDER_TECHNIQUE_PCSS_SAVSM;
		}

	private:
		const int32_t _pcssLookupTableSize;
		Ptr<Texture> _pcssLookupTable;

		void InitPCSSLookupTable();
	};
}

#endif