#include "ToyGE\RenderEngine\Effects\PointShadowDepthTechnique.h"
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
	PointShadowDepthTechnique::PointShadowDepthTechnique()
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"PointShadowDepth.xml");
	}

	void PointShadowDepthTechnique::RenderDepth(
		const Ptr<Texture> & shadowMap,
		const Ptr<LightComponent> & light,
		const Ptr<RenderSharedEnviroment> & sharedEnv)
	{
		auto pointLight = std::static_pointer_cast<PointLightComponent>(light);

		auto lightPosXM = XMLoadFloat3(&pointLight->GetPos());
		std::vector<XMFLOAT4X4> viewArray(6);
		std::vector<XMFLOAT4X4> viewProjArray(6);
		static std::vector<XMFLOAT3> viewDir =
		{
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT3(-1.0f, 0.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT3(0.0f, -1.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f)
		};
		static std::vector<XMFLOAT3> upDir =
		{
			XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT3(0.0f, 0.0f, -1.0f),
			XMFLOAT3(0.0f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f)
		};

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		RenderContextStateSave stateSave;
		rc->SaveState(
			RENDER_CONTEXT_STATE_DEPTHSTENCIL
			| RENDER_CONTEXT_STATE_RENDERTARGETS
			| RENDER_CONTEXT_STATE_VIEWPORT, stateSave);

		//Create Temp DepthStencilTex
		TextureDesc tempDSDesc = shadowMap->Desc();
		tempDSDesc.format = RENDER_FORMAT_D32_FLOAT;
		tempDSDesc.arraySize = 1;
		tempDSDesc.type = TEXTURE_2D;
		tempDSDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		auto tempDS = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(tempDSDesc);
		rc->SetDepthStencil(tempDS->CreateTextureView());

		//Set Viewport
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(shadowMap->Desc().width);
		vp.height = static_cast<float>(shadowMap->Desc().height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		float maxDist = pointLight->MaxDistance();
		_fx->VariableByName("pointMaxDist")->AsScalar()->SetValue(&maxDist, sizeof(maxDist));

		PerspectiveCamera camera = PerspectiveCamera(XM_PIDIV2, 1.0f, 0.1f, pointLight->MaxDistance());
		auto projXM = XMLoadFloat4x4(&camera.ProjMatrix());
		for (int32_t i = 0; i < 6; ++i)
		{
			//Compute ViewProjMatrix
			camera.LookTo(pointLight->GetPos(), viewDir[i], upDir[i]);

			//Cull
			std::vector<Ptr<Cullable>> renderCull;
			auto renderCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
			renderCuller->Cull(camera.GetFrustum(), renderCull);
			std::vector<Ptr<RenderComponent>> renderElements;
			for (auto & elem : renderCull)
				renderElements.push_back(std::static_pointer_cast<RenderComponent>(elem));

			_fx->VariableByName("view")->AsScalar()->SetValue(&camera.ViewMatrix());
			_fx->VariableByName("proj")->AsScalar()->SetValue(&camera.ProjMatrix());

			rc->ClearDepthStencil(1.0f, 0);
			rc->SetRenderTargets({ shadowMap->CreateTextureView(0, 1, i, 1) }, 0);
			//rc->ClearRenderTargets({ 1.0f, 1.0f, 1.0f, 1.0f });

			//Render Depth
			for (auto & elem : renderElements)
			{
				Ptr<RenderTechnique> technique;
				if (elem->GetMaterial()->NumTextures(MATERIAL_TEXTURE_OPACITYMASK) > 0)
				{
					//technique = _fx->TechniqueByName("PointShadowDepth_OPACITYTEX");
					_fx->SetExtraMacros({ { "OPACITY_TEX", "" } });
					auto opacityTex = elem->GetMaterial()->AcquireRender()->GetTexture(MATERIAL_TEXTURE_OPACITYMASK, 0);
					_fx->VariableByName("opacityTex")->AsShaderResource()->SetValue(opacityTex->CreateTextureView());
				}
				else
				{
					_fx->SetExtraMacros({});
				}
				technique = _fx->TechniqueByName("PointShadowDepth");

				_fx->VariableByName("world")->AsScalar()->SetValue(&elem->GetTransformMatrix());
				rc->SetRenderInput(elem->GetMesh()->AcquireRender()->GetRenderInput());
				for (int32_t passIndex = 0; passIndex < technique->NumPasses(); ++passIndex)
				{
					technique->PassByIndex(passIndex)->Bind();
					rc->DrawIndexed();
					technique->PassByIndex(passIndex)->UnBind();
				}
			}
		}

		rc->RestoreState(stateSave);

		tempDS->Release();
	}

	void PointShadowDepthTechnique::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light)
	{
		auto pointLight = std::static_pointer_cast<PointLightComponent>(light);
		float pointMaxDist = pointLight->MaxDistance();
		fx->VariableByName("pointMaxDist")->AsScalar()->SetValue(&pointMaxDist, sizeof(pointMaxDist));
	}
}