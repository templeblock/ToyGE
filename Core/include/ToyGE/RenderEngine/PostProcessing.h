#pragma once
#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class RenderAction;

	class TOYGE_CORE_API PostProcessing
	{
	public:
		virtual ~PostProcessing() = default;

		void AddRender(const Ptr<RenderAction> & postRender)
		{
			_postRenders.push_back(postRender);
		}

		void RemoveRender(const Ptr<RenderAction> & postRender)
		{
			auto renderFind = std::find(_postRenders.begin(), _postRenders.end(), postRender);
			if (renderFind != _postRenders.end())
				_postRenders.erase(renderFind);
		}

		virtual void PreTAASetup(const Ptr<class RenderView> & view);

		virtual void Render(const Ptr<class RenderView> & view);

	protected:
		std::vector<Ptr<RenderAction>> _postRenders;
	};
}

#endif
