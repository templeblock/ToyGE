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

		auto cam = view->GetCamera();
		auto camPos = cam->GetPos();
		float camFar = cam->GetFar();
		auto viewMat = cam->GetViewMatrix();
		auto projMat = cam->GetProjMatrix();
		auto viewMatXM = XMLoadFloat4x4(&viewMat);
		auto projMatXM = XMLoadFloat4x4(&projMat);
		auto scaleMatXM = XMMatrixIdentity();
		auto translationMatXM = XMMatrixTranslation(camPos.x, camPos.y, camPos.z);
		auto viewProjXM = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(scaleMatXM, translationMatXM), viewMatXM), projMatXM);
		XMFLOAT4X4 viewProj;
		XMStoreFloat4x4(&viewProj, viewProjXM);

		auto vs = Shader::FindOrCreate<SkyBoxVS>();
		auto ps = Shader::FindOrCreate<SkyBoxPS>();

		vs->SetScalar("transform", viewProj);

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