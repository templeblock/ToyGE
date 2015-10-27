#include "ToyGE\RenderEngine\Effects\SpotShadowDepthTechnique.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\Mesh.h"

namespace ToyGE
{
	SpotShadowDepthTechnique::SpotShadowDepthTechnique()
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"SpotShadowDepth.xml");
	}

	void SpotShadowDepthTechnique::RenderDepth(
		const Ptr<Texture> & shadowMap,
		const Ptr<LightComponent> & light,
		const Ptr<RenderSharedEnviroment> & sharedEnv,
		const std::array<Ptr<Texture>, 3> & rsm)
	{
		UpdateDepthRasterizerState();

		auto spotLight = std::static_pointer_cast<SpotLightComponent>(light);

		auto lightPosXM = XMLoadFloat3(&spotLight->GetPos());
		auto lightDirXM = XMLoadFloat3(&spotLight->Direction());
		XMFLOAT3 lightUp = abs(spotLight->Direction().y) == 1.0f ? XMFLOAT3(1.0f, 0.0f, 0.0f) : XMFLOAT3(0.0f, 1.0f, 0.0f);
		auto lightUpXM = XMLoadFloat3(&lightUp);
		auto viewXM = XMMatrixLookToLH(lightPosXM, lightDirXM, lightUpXM);
		auto spotCam = std::make_shared<PerspectiveCamera>(2.0f * spotLight->MaxAngle(), 1.0f, 0.1f, spotLight->MaxDistance());
		auto projXM = XMLoadFloat4x4(&spotCam->ProjMatrix());
		auto viewProjXM = XMMatrixMultiply(viewXM, projXM);

		XMFLOAT4X4 view;
		XMStoreFloat4x4(&view, viewXM);
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&proj, projXM);
		XMFLOAT4X4 viewProj;
		XMStoreFloat4x4(&viewProj, viewProjXM);

		_lightViewMat = view;
		_lightProjMat = proj;

		_fx->VariableByName("spotView")->AsScalar()->SetValue(&view);
		_fx->VariableByName("spotViewProj")->AsScalar()->SetValue(&viewProj);
		float2 spotNearFar = float2(0.1f, spotLight->MaxDistance());
		_fx->VariableByName("spotNearFar")->AsScalar()->SetValue(&spotNearFar, sizeof(spotNearFar));

		std::vector<Ptr<Cullable>> renderCull;
		auto renderCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
		renderCuller->Cull(spotLight->GetBoundsFrustum(), renderCull);
		std::vector<Ptr<RenderComponent>> renderElements;
		for (auto & elem : renderCull)
			renderElements.push_back(std::static_pointer_cast<RenderComponent>(elem));

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		TextureDesc tempDSDesc = shadowMap->Desc();
		tempDSDesc.format = RENDER_FORMAT_D32_FLOAT;
		tempDSDesc.type = TEXTURE_2D;
		tempDSDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		auto tempDS = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(tempDSDesc);

		auto preDepthStencil = rc->GetDepthStencil();
		rc->SetDepthStencil(tempDS->CreateTextureView(0, 1, 0, 1));
		rc->ClearDepthStencil(1.0f, 0);

		auto preRTs = rc->GetRenderTargets();
		rc->SetRenderTargets({ shadowMap->CreateTextureView(0, 1, 0, 1) }, 0);
		//rc->ClearRenderTargets({ 1.0f, 1.0f, 1.0f, 1.0f });

		auto preViewport = rc->GetViewport();
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(shadowMap->Desc().width);
		vp.height = static_cast<float>(shadowMap->Desc().height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		//auto rc = Global::GetRenderEngine()->GetRenderContext();
		//auto technique = _fx->TechniqueByName("SpotShadowDepth");
		for (auto & elem : renderElements)
		{
			Ptr<RenderTechnique> technique;
			if (elem->GetMaterial()->NumTextures(MATERIAL_TEXTURE_OPACITYMASK) > 0)
			{
				//technique = _fx->TechniqueByName("SpotShadowDepth_OPACITYTEX");
				_fx->SetExtraMacros({ { "OPACITY_TEX", "" } });
				auto opacityTex = elem->GetMaterial()->AcquireRender()->GetTexture(MATERIAL_TEXTURE_OPACITYMASK, 0);
				_fx->VariableByName("opacityTex")->AsShaderResource()->SetValue(opacityTex->CreateTextureView());
			}
			else
			{
				_fx->SetExtraMacros({});
			}
			technique = _fx->TechniqueByName("SpotShadowDepth");

			//obj->Render(fx, fx->TechniqueByName(techniqueName));
			_fx->VariableByName("world")->AsScalar()->SetValue(&elem->GetTransformMatrix());
			rc->SetRenderInput(elem->GetMesh()->AcquireRender()->GetRenderInput());
			for (int32_t passIndex = 0; passIndex < technique->NumPasses(); ++passIndex)
			{
				technique->PassByIndex(passIndex)->Bind();
				rc->SetRasterizerState(_depthRenderRS);
				rc->DrawIndexed();
				technique->PassByIndex(passIndex)->UnBind();
			}
		}

		rc->SetDepthStencil(preDepthStencil);
		rc->SetRenderTargets(preRTs, 0);
		rc->SetViewport(preViewport);

		tempDS->Release();
	}

	void SpotShadowDepthTechnique::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light)
	{
		auto spotLight = std::static_pointer_cast<SpotLightComponent>(light);
		//float spotMaxDist = spotLight->MaxDistance();
		float2 spotNearFar = float2(0.1f, spotLight->MaxDistance());
		fx->VariableByName("spotNearFar")->AsScalar()->SetValue(&spotNearFar, sizeof(spotNearFar));
		//fx->VariableByName("spotMaxDist")->AsScalar()->SetValue(&spotMaxDist, sizeof(spotMaxDist));
		fx->VariableByName("spotView")->AsScalar()->SetValue(&_lightViewMat);
		fx->VariableByName("spotProj")->AsScalar()->SetValue(&_lightProjMat);
	}
}