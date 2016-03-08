#include "ToyGE\RenderEngine\Effects\AmbientPanoramicMap.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Camera.h"

namespace ToyGE
{
	void AmbientPanoramicMap::Render(const Ptr<RenderView> & view)
	{
		if (!_tex)
			return;

		auto translationMat = translation(view->GetCamera()->GetPos());
		auto transformMat = mul(translationMat, view->GetCamera()->GetViewProjMatrix());

		auto vs = Shader::FindOrCreate<AmbientPanoramicMapVS>();
		auto ps = Shader::FindOrCreate<AmbientPanoramicMapPS>();

		view->BindShaderParams(ps);

		vs->SetScalar("transform", transformMat);

		ps->SetSRV("ambientPanoramicMap", _tex->GetShaderResourceView());
		ps->SetSampler("inSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_LINEAR, TEXTURE_ADDRESS_CLAMP>::Get());

		vs->Flush();
		ps->Flush();

		AmbientMap::Render(view);
	}
}