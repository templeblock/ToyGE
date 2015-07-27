#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	class RenderInput;

	class TOYGE_CORE_API SkyBox : public RenderAction
	{
	public:
		SkyBox();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		void SetTexture(const Ptr<Texture> & tex)
		{
			_tex = tex;
		}

	private:
		Ptr<RenderEffect> _skyBoxFX;
		Ptr<RenderInput> _sphereInput;
		Ptr<Texture> _tex;
	};
}

#endif