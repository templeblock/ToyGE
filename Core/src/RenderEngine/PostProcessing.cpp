#include "ToyGE\RenderEngine\PostProcessing.h"
#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	void PostProcessing::Render(const Ptr<class RenderView> & view)
	{
		for (auto & render : _postRenders)
		{
			if (render->GetEnable())
				render->Render(view);
		}
	}
}