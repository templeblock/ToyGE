#include "ToyGE\RenderEngine\Effects\SkyBox.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	SkyBox::SkyBox()
	{
		_sphereMesh = CommonMesh::CreateSphere(1.0f, 25);
	}

	void SkyBox::Render(const Ptr<RenderTargetView> & target, const Ptr<DepthStencilView> & dsv, const Ptr<RenderView> & view)
	{
		if (!_tex)
			return;

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto translationMat = translation(view->GetCamera()->GetPos());
		auto transformMat = mul(translationMat, view->GetCamera()->GetViewProjMatrix());

		auto vs = Shader::FindOrCreate<SkyBoxVS>();
		auto ps = Shader::FindOrCreate<SkyBoxPS>();

		vs->SetScalar("transform", transformMat);

		ps->SetSRV("skyBoxTex", _tex->GetShaderResourceView(0, 0, 0, 0, true));
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		vs->Flush();
		ps->Flush();

		rc->SetViewport(GetTextureQuadViewport(target->GetResource()->Cast<Texture>()));

		rc->SetRenderTargets({ target });
		rc->SetDepthStencil(dsv);

		rc->SetRasterizerState(RasterizerStateTemplate<FILL_SOLID, CULL_NONE>::Get());
		rc->SetDepthStencilState(DepthStencilStateTemplate<true, DEPTH_WRITE_ZERO, COMPARISON_LESS_EQUAL>::Get());

		rc->SetVertexBuffer(_sphereMesh->GetRenderData()->GetMeshElements()[0]->GetVertexBuffer());
		rc->SetIndexBuffer(_sphereMesh->GetRenderData()->GetMeshElements()[0]->GetIndexBuffer());
		rc->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		rc->DrawIndexed(0, 0);

		rc->SetRasterizerState(nullptr);
		rc->SetDepthStencilState(nullptr);
	}
}