#include "ToyGE\RenderEngine\Effects\SkyBox.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\Model.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderTechnique.h"
#include "ToyGE\RenderEngine\RenderPass.h"
#include "ToyGE\RenderEngine\RenderEffectVariable.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"

namespace ToyGE
{
	SkyBox::SkyBox()
	{
		_skyBoxFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"SkyBox.xml");
		/*auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource("sphere.3ds");
		RenderMesh renderMesh = RenderMesh(model->GetMesh(0));
		_sphereInput = renderMesh.RenderInput();*/
		auto sphereMesh = CommonMesh::CreateSphere(1.0f, 25);
		sphereMesh->InitRenderData();
		_sphereInput = sphereMesh->AcquireRender()->GetRenderInput();
	}

	void SkyBox::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto cam = sharedEnviroment->GetView()->GetCamera();
		auto camPos = cam->Pos();
		float camFar = cam->Far();
		auto viewMat = cam->ViewMatrix();
		auto projMat = cam->ProjMatrix();
		auto viewMatXM = XMLoadFloat4x4(&viewMat);
		auto projMatXM = XMLoadFloat4x4(&projMat);
		//auto scaleMatXM = XMMatrixScaling(camFar, camFar, camFar);
		auto scaleMatXM = XMMatrixIdentity();
		auto translationMatXM = XMMatrixTranslation(camPos.x, camPos.y, camPos.z);
		auto viewProjXM = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(scaleMatXM, translationMatXM), viewMatXM), projMatXM);
		XMFLOAT4X4 viewProj;
		XMStoreFloat4x4(&viewProj, viewProjXM);

		_skyBoxFX->VariableByName("skyboxTex")->AsShaderResource()->SetValue(_tex->CreateTextureView_Cube(0, 0, 0, 1));
		_skyBoxFX->VariableByName("camPos")->AsScalar()->SetValue(&camPos, sizeof(camPos));
		_skyBoxFX->VariableByName("viewProj")->AsScalar()->SetValue(&viewProj);

		auto rawDepth = sharedEnviroment->ParamByName(CommonRenderShareName::RawDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		rc->SetRenderTargets({ sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView() }, 0);
		rc->SetDepthStencil(rawDepth->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		rc->SetRenderInput(_sphereInput);
		_skyBoxFX->TechniqueByIndex(0)->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_skyBoxFX->TechniqueByIndex(0)->PassByIndex(0)->UnBind();
	}
}