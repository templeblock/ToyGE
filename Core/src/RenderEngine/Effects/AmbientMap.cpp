#include "ToyGE\RenderEngine\Effects\AmbientMap.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Effects\AmbientCubeMap.h"
#include "ToyGE\RenderEngine\Effects\AmbientPanoramicMap.h"
#include "ToyGE\RenderEngine\Effects\AmbientHemiPanoramicMap.h"

namespace ToyGE
{
	Ptr<AmbientMap> AmbientMap::Create(AmbientMapType type)
	{
		switch (type)
		{
		case ToyGE::AM_CUBEMAP:
			return std::make_shared<AmbientCubeMap>();
		case ToyGE::AM_PANORAMIC:
			return std::make_shared<AmbientPanoramicMap>();
		case ToyGE::AM_HEMIPANORAMIC:
			return std::make_shared<AmbientHemiPanoramicMap>();
		default:
			break;
		}
		return nullptr;
	}

	void AmbientMap::Render(const Ptr<class RenderView> & view)
	{
		if (!_tex)
			return;

		if (!_sphereMesh)
			_sphereMesh = CommonMesh::CreateSphere(1.0f, 50);

		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetViewport(GetTextureQuadViewport(sceneTex));

		rc->SetRenderTargets({ sceneTex->GetRenderTargetView(0, 0, 1) });
		rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		rc->SetRasterizerState(RasterizerStateTemplate<FILL_SOLID, CULL_NONE>::Get());
		rc->SetDepthStencilState(DepthStencilStateTemplate<true, DEPTH_WRITE_ZERO, COMPARISON_LESS_EQUAL>::Get());

		rc->SetVertexBuffer(_sphereMesh->GetRenderData()->GetMeshElements()[0]->GetVertexBuffer());
		rc->SetIndexBuffer(_sphereMesh->GetRenderData()->GetMeshElements()[0]->GetIndexBuffer());
		rc->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		rc->DrawIndexed(0, 0);

		rc->SetRasterizerState(nullptr);
		rc->SetDepthStencilState(nullptr);
	}

	/*void AmbientMap::Bind(const Ptr<class RenderView> & view, Ptr<Shader> & vs, Ptr<Shader> & ps)
	{
		if (!_sphereMesh)
			_sphereMesh = CommonMesh::CreateSphere(1.0f, 25);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		

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
	}*/
}