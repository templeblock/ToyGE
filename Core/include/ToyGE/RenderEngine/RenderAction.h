#pragma once
#ifndef RENDERACTION_H
#define RENDERACTION_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"


namespace ToyGE
{
	class TOYGE_CORE_API RenderAction
	{
	public:
		RenderAction()
			: _bEnable(true)
		{

		}

		virtual ~RenderAction() = default;

		virtual void PreTAASetup(const Ptr<RenderView> & view){}

		virtual void Render(const Ptr<RenderView> & view) = 0;

		CLASS_SET(Enable, bool, _bEnable);
		CLASS_GET(Enable, bool, _bEnable);

	protected:
		bool _bEnable;
	};


}

#endif