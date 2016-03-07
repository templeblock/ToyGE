#pragma once
#ifndef ENVIRONMENTREFLECTIONRENDERER_H
#define ENVIRONMENTREFLECTIONRENDERER_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	DECLARE_SHADER(, EnvironmentReflectionRenderingCS, SHADER_CS, "EnvironmentReflectionRendering", "EnvironmentReflectionRenderingCS", SM_5);

	class TOYGE_CORE_API EnvironmentReflectionRenderer : public RenderAction
	{
	public:
		virtual void Render(const Ptr<RenderView> & view) override;

	private:

	};
}

#endif
