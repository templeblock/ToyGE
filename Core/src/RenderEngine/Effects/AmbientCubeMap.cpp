#include "ToyGE\RenderEngine\Effects\AmbientCubeMap.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	void AmbientCubeMap::Render(const Ptr<class RenderView> & view)
	{
		if (!_tex)
			return;

		auto translationMat = translation(view->GetCamera()->GetPos());
		auto transformMat = mul(translationMat, view->GetCamera()->GetViewProjMatrix());

		auto vs = Shader::FindOrCreate<AmbientCubeMapVS>();
		auto ps = Shader::FindOrCreate<AmbientCubeMapPS>();

		vs->SetScalar("transform", transformMat);

		ps->SetSRV("ambientCubeMap", _tex->GetShaderResourceView(0, 0, 0, 0, true));
		ps->SetSampler("inSampler", SamplerTemplate<>::Get());

		vs->Flush();
		ps->Flush();

		AmbientMap::Render(view);
	}
}