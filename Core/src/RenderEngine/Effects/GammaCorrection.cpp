#include "ToyGE\RenderEngine\Effects\GammaCorrection.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	GammaCorrection::GammaCorrection()
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"GammaCorrection.xml");
	}

	void GammaCorrection::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);

		//auto gammaInTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(targetTex->Desc());
		//targetTex->CopyTo(gammaInTex, 0, 0, 0, 0, 0, 0, 0);

		_fx->VariableByName("inTex")->AsShaderResource()->SetValue(sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("GammaCorrection"));

		sharedEnviroment->GetView()->FlipRenderTarget();
	}
}