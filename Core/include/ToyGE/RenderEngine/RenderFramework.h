#pragma once
#ifndef RENDERFRAMEWORK_H
#define RENDERFRAMEWORK_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class SceneRenderer;

	class TOYGE_CORE_API RenderFramework
	{
	public:
		virtual ~RenderFramework() = default;

		virtual void Init() = 0;

		virtual void Render() = 0;

		CLASS_GET(SceneRenderer, Ptr<SceneRenderer>, _sceneRenderer);

	protected:
		Ptr<SceneRenderer> _sceneRenderer;
	};
}

#endif