#pragma once
#ifndef DEFERREDRENDERFRAMEWORK_H
#define DEFERREDRENDERFRAMEWORK_H

#include "ToyGE\RenderEngine\RenderFramework.h"

namespace ToyGE
{
	class DebugInfoRender;

	class TOYGE_CORE_API DeferredRenderFramework : public RenderFramework
	{
	public:
		DeferredRenderFramework();

		void Init() override;

		void Render() override;

	private:
		Ptr<DebugInfoRender> _debugInfoRender;
	};
}

#endif