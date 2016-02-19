#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	DECLARE_SHADER(, SkyBoxVS, SHADER_VS, "SkyBox", "SkyBoxVS", SM_4);
	DECLARE_SHADER(, SkyBoxPS, SHADER_PS, "SkyBox", "SkyBoxPS", SM_4);

	class Texture;

	class TOYGE_CORE_API SkyBox
	{
	public:
		SkyBox();

		void Render(
			const Ptr<class RenderTargetView> & target, 
			const Ptr<class DepthStencilView> & dsv, 
			const Ptr<class RenderView> & view);

		CLASS_SET(Texture, Ptr<Texture>, _tex)
		CLASS_GET(Texture, Ptr<Texture>, _tex)

	private:
		Ptr<class Mesh> _sphereMesh;
		Ptr<Texture> _tex;
	};
}

#endif