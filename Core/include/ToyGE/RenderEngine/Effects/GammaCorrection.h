#pragma once
#ifndef GAMMACORRECTION_H
#define GAMMACORRECTION_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	class TOYGE_CORE_API GammaCorrection : public RenderAction
	{
	public:
		GammaCorrection();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _fx;
	};
}

#endif