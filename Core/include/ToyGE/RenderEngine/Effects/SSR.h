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

	private:
		Ptr<RenderEffect> _fx;

		void InitDither();

		Ptr<Texture> BuildHZB(const Ptr<Texture> & depthTex);
	};
}

#endif