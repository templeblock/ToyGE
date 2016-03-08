#include "ToyGE\RenderEngine\Effects\AmbientHemiPanoramicMap.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Camera.h"

namespace ToyGE
{
	void AmbientHemiPanoramicMap::Render(const Ptr<RenderView> & view)
	{
		if (!_tex)
			return;

		auto translationMat = translation(view->GetCamera()->GetPos());
		auto transformMat = mul(translationMat, view->GetCamera()->GetViewProjMatrix());

		auto vs = Shader::FindOrCreate<AmbientHemiPanoramicMapVS>();
		auto ps = Shader::FindOrCreate<AmbientHemiPanoramicMapPS>();

		view->BindShaderParams(ps);

		vs->SetScalar("transform", transformMat);

		ps->SetSRV("ambientPanoramicMap", _tex->GetShaderResourceView());

		ps->SetSampler("inSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_LINEAR, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_BORDER>::Get());

		vs->Flush();
		ps->Flush();

		AmbientMap::Render(view);
		/*auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetDepthStencilState(DepthStencilStateTemplate<false, DEPTH_WRITE_ZERO, COMPARISON_LESS_EQUAL,
			true, 0xff, 0xff, STENCIL_OP_KEEP, STENCIL_OP_KEEP, STENCIL_OP_KEEP, COMPARISON_EQUAL>::Get(), 0);

		DrawQuad({ sceneTex->GetRenderTargetView(0, 0, 1) },
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		rc->SetDepthStencilState(nullptr);*/
	}
}