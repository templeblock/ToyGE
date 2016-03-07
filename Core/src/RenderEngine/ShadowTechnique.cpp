#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\Blur.h"

namespace ToyGE
{
	void ShadowTechnique::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		outMacros["ENABLE_SHADOW"] = "1";
	}

	void ShadowTechnique::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		
	}

	void ShadowTechnique::PrepareShadow(const Ptr<RenderView> & view)
	{
	}

	void ShadowTechnique::PrepareRSM(const Ptr<RenderView> & view)
	{
	}


	void DepthDrawingPolicy::BindView(const Ptr<class RenderView> & view)
	{
		_view = view;
	}

	void DepthDrawingPolicy::BindMaterial(const Ptr<class Material> & mat)
	{
		_mat = mat;
	}

	void DepthDrawingPolicy::Draw(const Ptr<class RenderComponent> & renderComponent)
	{
		std::map<String, String> macros;

		bool bWithOpacityMask = false;

		if (_mat)
		{
			_mat->BindDepthMacros(macros);
			bWithOpacityMask = _mat->GetTexture(MAT_TEX_OPACITYMASK).size() > 0;
		}
		else
		{
			Material().BindDepthMacros(macros);
		}

		auto meshElement = renderComponent->GetMeshElement();
		if (meshElement)
			meshElement->BindDepthMacros(bWithOpacityMask, macros);

		auto vs = Shader::FindOrCreate<DepthOnlyVS>(macros);
		auto ps = Shader::FindOrCreate<DepthOnlyPS>(macros);

		_view->BindShaderParams(vs);
		renderComponent->BindShaderParams(vs);

		if (_mat)
			_mat->BindDepthShaderParams(ps);
		else
			Material().BindDepthShaderParams(ps);

		ps->SetSampler("bilinearSampler",
			SamplerTemplate<
			FILTER_MIN_MAG_MIP_LINEAR,
			TEXTURE_ADDRESS_WRAP,
			TEXTURE_ADDRESS_WRAP,
			TEXTURE_ADDRESS_WRAP >::Get());

		vs->Flush();
		ps->Flush();

		meshElement->DrawDepth(bWithOpacityMask);
	}

	void ShadowTechnique::RenderDepth(const Ptr<DepthStencilView> & dsv, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & viewPosTarget)
	{
		// Init draw list
		auto drawList = std::make_shared<PrimitiveDrawList>();

		auto objsCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
		std::vector<Ptr<Cullable>> objsCulled;
		//objsCuller->Cull(_depthRenderView->GetCamera()->GetFrustum(), objsCulled);
		view->GetCamera()->Cull(objsCuller, objsCulled);
		for (auto & obj : objsCulled)
		{
			auto renderObj = std::static_pointer_cast<RenderComponent>(obj);
			if(renderObj->IsCastShadows())
				drawList->AddRenderComponent(renderObj);
		}

		//auto opaqueDrawList = std::make_shared<PrimitiveDrawList>();
		//auto translucentDrawList = std::make_shared<PrimitiveDrawList>();
		//for (auto & drawBatchPair : drawList->drawBatches)
		//{
		//	auto & mat = drawBatchPair.first;
		//	if (!mat->IsTranslucent())
		//	{
		//		opaqueDrawList->drawBatches[mat] = drawBatchPair.second;
		//	}
		//	else
		//	{
		//		translucentDrawList->drawBatches[mat] = drawBatchPair.second;
		//	}
		//}

		// Render depth
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetDepthStencil(dsv);
		rc->ClearDepthStencil(1.0f, 0);

		rc->SetDepthStencilState(nullptr);
		rc->SetBlendState(nullptr);
		rc->SetRasterizerState(nullptr);

		rc->SetViewport(view->GetViewport());

		rc->SetRenderTargets({});

		auto drawingPolicy = std::make_shared<DepthDrawingPolicy>();

		drawList->Draw(drawingPolicy, view);
	}

	class RSMDrawingPolicy : public DrawingPolicy
	{
	public:
		Ptr<RenderView> _view;
		Ptr<Material> _mat;
		Ptr<LightComponent> _light;

		virtual void BindView(const Ptr<class RenderView> & view) override
		{
			_view = view;
		}

		virtual void BindMaterial(const Ptr<class Material> & mat) override
		{
			_mat = mat;
		}

		virtual void Draw(const Ptr<class RenderComponent> & renderComponent) override
		{
			std::map<String, String> macros;

			if (_mat)
				_mat->BindMacros(macros);
			else
				Material().BindMacros(macros);

			auto meshElement = renderComponent->GetMeshElement();
			if (meshElement)
				meshElement->BindMacros(macros);

			_light->BindMacros(false, nullptr, macros);

			auto vs = Shader::FindOrCreate<RenderRSMVS>(macros);
			auto ps = Shader::FindOrCreate<RenderRSMPS>(macros);

			_view->BindShaderParams(vs);
			renderComponent->BindShaderParams(vs);
			_light->BindShaderParams(ps, false, nullptr);

			if (_mat)
				_mat->BindShaderParams(ps);
			else
				Material().BindShaderParams(ps);

			ps->SetScalar("gamma", Global::GetRenderEngine()->GetGamma());

			ps->SetSampler("bilinearSampler",
				SamplerTemplate<
				FILTER_MIN_MAG_MIP_LINEAR,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP >::Get());

			vs->Flush();
			ps->Flush();

			meshElement->Draw();
		}
	};

	void ShadowTechnique::RenderRSM(
		const Ptr<RenderView> & view,
		const Ptr<RenderTargetView> & rsmRadiance,
		const Ptr<DepthStencilView> & rsmDepth,
		const Ptr<RenderTargetView> & rsmNormal)
	{
		// Init draw list
		auto drawList = std::make_shared<PrimitiveDrawList>();

		auto objsCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
		std::vector<Ptr<Cullable>> objsCulled;
		view->GetCamera()->Cull(objsCuller, objsCulled);
		for (auto & obj : objsCulled)
		{
			auto renderObj = std::static_pointer_cast<RenderComponent>(obj);
			if (renderObj->IsCastShadows())
				drawList->AddRenderComponent(renderObj);
		}

		// Render RSM
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ rsmRadiance, rsmNormal });

		rc->SetDepthStencil(rsmDepth);
		rc->ClearDepthStencil(1.0f, 0);

		rc->SetDepthStencilState(nullptr);
		rc->SetBlendState(nullptr);
		rc->SetRasterizerState(nullptr);

		rc->SetViewport(view->GetViewport());

		auto drawingPolicy = std::make_shared<RSMDrawingPolicy>();
		drawingPolicy->_light = GetLight();

		drawList->Draw(drawingPolicy, view);
	}


	/**
	*		SpotLightShadow
	*/

	SpotLightShadow::SpotLightShadow()
	{
		_depthRenderView = std::make_shared<RenderView>();
		_depthRenderView->sceneRenderingConfig.bTAA = false;

		_spotShadowViewCB = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_CONSTANT;
		bufDesc.cpuAccess = CPU_ACCESS_WRITE;
		bufDesc.elementSize = sizeof(_spotShadowView);
		bufDesc.numElements = 1;
		bufDesc.bStructured = false;
		_spotShadowViewCB->SetDesc(bufDesc);
		_spotShadowViewCB->Init(nullptr);
	}

	void SpotLightShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		ShadowTechnique::PrepareShadow(view);

		// Init shadow map texture
		TextureDesc depthDesc;
		depthDesc.width = _shadowMapSize.x();
		depthDesc.height = _shadowMapSize.y();
		depthDesc.depth = 1;
		depthDesc.arraySize = 1;
		depthDesc.mipLevels = 1;
		depthDesc.bCube = false;
		depthDesc.cpuAccess = 0;
		depthDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_DEPTH_STENCIL;
		depthDesc.format = RENDER_FORMAT_R24G8_TYPELESS;
		depthDesc.sampleCount = 1;
		depthDesc.sampleQuality = 0;

		auto depthTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, depthDesc });
		auto depthTex = depthTexRef->Get()->Cast<Texture>();

		//PooledTextureRef viewPosTexRef;
		//Ptr<Texture> viewPosTex;
		//if (bOutputViewPos)
		//{
		//	depthDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		//	depthDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		//	viewPosTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, depthDesc });
		//	viewPosTex = viewPosTexRef->Get()->Cast<Texture>();
		//}

		// Init camera
		auto spotLight = std::static_pointer_cast<SpotLightComponent>(GetLight());
		//auto lightPos = spotLight->GetPos();
		//auto lightDir = spotLight->Direction();
		float3 lightUp = abs(spotLight->Direction().y()) == 1.0f ? float3(1.0f, 0.0f, 0.0f) : float3(0.0f, 1.0f, 0.0f);
		////auto lightUpXM = XMLoadFloat3(&lightUp);
		//auto viewXM = look_at_lh(lightPos, lightPos + lightDir, lightUp);
		auto spotCamera = std::make_shared<PerspectiveCamera>(2.0f * spotLight->MaxAngle(), 1.0f, 0.1f, spotLight->MaxDistance());
		spotCamera->LookTo(spotLight->GetPos(), spotLight->Direction(), lightUp);
		/*auto projXM = XMLoadFloat4x4(&spotCamera->GetProjMatrix());
		auto viewProjXM = XMMatrixMultiply(viewXM, projXM);*/

		// Update cb
		_spotShadowView.shadowWorldToViewMatrix[0] = spotCamera->GetViewMatrix();
		_spotShadowView.shadowViewToClipMatrix[0] = spotCamera->GetProjMatrix();
		_spotShadowView.shadowWorldToClipMatrix[0] = spotCamera->GetViewProjMatrix();
		_spotShadowView.shadowViewNear = spotCamera->GetNear();
		_spotShadowView.shadowViewFar = spotCamera->GetFar();
		_spotShadowView.shadowViewLength = _spotShadowView.shadowViewFar - _spotShadowView.shadowViewNear;
		_spotShadowView.shadowViewSize = float2((float)_shadowMapSize.x(), (float)_shadowMapSize.y());
		_spotShadowView.shadowInvViewSize = 1.0f / _spotShadowView.shadowViewSize;
		auto mappedData = _spotShadowViewCB->Map(MAP_WRITE_DISCARD);
		memcpy(mappedData.pData, &_spotShadowView, sizeof(_spotShadowView));
		_spotShadowViewCB->UnMap();


		// Render depth
		_depthRenderView->SetViewport(GetTextureQuadViewport(depthTex));
		_depthRenderView->SetCamera(spotCamera);
		_depthRenderView->UpdateParamsBuffer();
		/*if (bOutputViewPos)
		{
			RenderDepth(depthTex->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT), _depthRenderView, viewPosTex->GetRenderTargetView(0, 0, 1));
			_shadowMaps[nullptr] = depthTexRef;
			_viewPosMaps[nullptr] = viewPosTexRef;
		}
		else
		{*/
		RenderDepth(depthTex->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT), _depthRenderView);
		_shadowMaps[nullptr] = depthTexRef;
		//}

	}

	void SpotLightShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		ShadowTechnique::BindMacros(view, outMacros);

		outMacros["NUM_SHADOW_VIEWS"] = "1";
	}

	void SpotLightShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		ShadowTechnique::BindShaderParams(shader, view);

		shader->SetSRV("shadowMap", _shadowMaps[nullptr]->Get()->Cast<Texture>()->GetShaderResourceView());
		shader->SetCB("cb_shadowView_shared", _spotShadowViewCB);
	}
	

	/**
	*		PointLightShadow
	*/

	PointLightShadow::PointLightShadow()
	{
		_depthRenderView = std::make_shared<RenderView>();
		_depthRenderView->sceneRenderingConfig.bTAA = false;

		_pointShadowViewCB = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_CONSTANT;
		bufDesc.cpuAccess = CPU_ACCESS_WRITE;
		bufDesc.elementSize = sizeof(_pointShadowView);
		bufDesc.numElements = 1;
		bufDesc.bStructured = false;
		_pointShadowViewCB->SetDesc(bufDesc);
		_pointShadowViewCB->Init(nullptr);
	}

	void PointLightShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		ShadowTechnique::PrepareShadow(view);

		// Init shadow map texture
		TextureDesc depthDesc;
		depthDesc.width = _shadowMapSize.x();
		depthDesc.height = _shadowMapSize.y();
		depthDesc.depth = 1;
		depthDesc.arraySize = 6;
		depthDesc.mipLevels = 1;
		depthDesc.bCube = false;
		depthDesc.cpuAccess = 0;
		depthDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_DEPTH_STENCIL;
		depthDesc.format = RENDER_FORMAT_R24G8_TYPELESS;
		depthDesc.sampleCount = 1;
		depthDesc.sampleQuality = 0;

		auto depthTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, depthDesc });
		auto depthTex = depthTexRef->Get()->Cast<Texture>();

		/*PooledTextureRef viewPosTexRef;
		Ptr<Texture> viewPosTex;
		if (bOutputViewPos)
		{
			depthDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
			depthDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
			viewPosTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, depthDesc });
			viewPosTex = viewPosTexRef->Get()->Cast<Texture>();
		}*/

		// Init camera
		static std::vector<float3> viewDir =
		{
			float3(1.0f, 0.0f, 0.0f),
			float3(-1.0f, 0.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, -1.0f, 0.0f),
			float3(0.0f, 0.0f, 1.0f),
			float3(0.0f, 0.0f, -1.0f)
		};
		static std::vector<float3> upDir =
		{
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 0.0f, -1.0f),
			float3(0.0f, 0.0f, 1.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f)
		};

		auto pointLight = std::static_pointer_cast<PointLightComponent>(GetLight());
		for (int32_t i = 0; i < 6; ++i)
		{
			/*const auto & lightPos = pointLight->GetPos());
			const auto & lightDir = viewDir[i];
			const auto & lightUp = upDir[i];
			auto viewXM = look_at_lh(lightPos, lightPos + lightDir, lightUp);*/
			auto pointCamera = std::make_shared<PerspectiveCamera>(PI_DIV2, 1.0f, 0.1f, pointLight->MaxDistance());
			pointCamera->LookTo(pointLight->GetPos(), viewDir[i], upDir[i]);
			/*auto projXM = XMLoadFloat4x4(&pointCamera->GetProjMatrix());
			auto viewProjXM = XMMatrixMultiply(viewXM, projXM);*/

			// Update cb
			_pointShadowView.shadowWorldToViewMatrix[i] = pointCamera->GetViewMatrix();
			_pointShadowView.shadowViewToClipMatrix[i] = pointCamera->GetProjMatrix();
			_pointShadowView.shadowWorldToClipMatrix[i] = pointCamera->GetViewProjMatrix();
			_pointShadowView.shadowViewNear = pointCamera->GetNear();
			_pointShadowView.shadowViewFar = pointCamera->GetFar();
			_pointShadowView.shadowViewLength = _pointShadowView.shadowViewFar - _pointShadowView.shadowViewNear;
			_pointShadowView.shadowViewSize = float2((float)_shadowMapSize.x(), (float)_shadowMapSize.y());
			_pointShadowView.shadowInvViewSize = 1.0f / _pointShadowView.shadowViewSize;


			// Render depth
			_depthRenderView->SetViewport(GetTextureQuadViewport(depthTex));
			_depthRenderView->SetCamera(pointCamera);
			_depthRenderView->UpdateParamsBuffer();

			/*if (bOutputViewPos)
			{
				RenderDepth(depthTex->GetDepthStencilView(0, i, 1, RENDER_FORMAT_D24_UNORM_S8_UINT), _depthRenderView, viewPosTex->GetRenderTargetView(0, i, 1));
			}
			else
			{*/
			RenderDepth(depthTex->GetDepthStencilView(0, i, 1, RENDER_FORMAT_D24_UNORM_S8_UINT), _depthRenderView);
			//}
		}
		auto mappedData = _pointShadowViewCB->Map(MAP_WRITE_DISCARD);
		memcpy(mappedData.pData, &_pointShadowView, sizeof(_pointShadowView));
		_pointShadowViewCB->UnMap();


		_shadowMaps[nullptr] = depthTexRef;
		/*if (bOutputViewPos)
		{
			_viewPosMaps[nullptr] = viewPosTexRef;
		}*/
	}

	void PointLightShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		ShadowTechnique::BindMacros(view, outMacros);
		
		outMacros["NUM_SHADOW_VIEWS"] = "6";
	}

	void PointLightShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		ShadowTechnique::BindShaderParams(shader, view);

		if(!_shadowMapBindAsCube)
			shader->SetSRV("shadowMap", _shadowMaps[nullptr]->Get()->Cast<Texture>()->GetShaderResourceView());
		else
			shader->SetSRV("shadowMapCube", _shadowMaps[nullptr]->Get()->Cast<Texture>()->GetShaderResourceView(0, 0, 0, 0, true));
		shader->SetCB("cb_shadowView_shared", _pointShadowViewCB);
		shader->SetScalar("shadowPointLightPos", GetLight()->GetPos());
	}

	/**
	*		CascadedShadow
	*/

	CascadedShadow::CascadedShadow()
	{
		for (auto & view : _depthRenderView)
		{
			view = std::make_shared<RenderView>();
			view->sceneRenderingConfig.bTAA = false;
		}
		for (auto & view : _rsmRenderView)
		{
			view = std::make_shared<RenderView>();
			view->sceneRenderingConfig.bTAA = false;
		}

		_cascadedShadowViewCB = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_CONSTANT;
		bufDesc.cpuAccess = CPU_ACCESS_WRITE;
		bufDesc.elementSize = sizeof(_cascadedShadowView);
		bufDesc.numElements = 1;
		bufDesc.bStructured = false;
		_cascadedShadowViewCB->SetDesc(bufDesc);
		_cascadedShadowViewCB->Init(nullptr);
	}

	static float2 ComputeSplit(
		const float2 & camNearFar,
		int32_t splitIndex,
		int32_t numSplits)
	{
		float splitLogUni_C = 0.5f;

		float camNear = camNearFar.x();
		float camFar = camNearFar.y();

		float nearZLog = camNear * pow((camFar / camNear), static_cast<float>(splitIndex) / static_cast<float>(numSplits));
		float nearZUni = camNear + (camFar - camNear) * (static_cast<float>(splitIndex) / static_cast<float>(numSplits));
		float nearZ = splitLogUni_C * nearZLog + (1.0f - splitLogUni_C) * nearZUni;

		float farZLog = camNear * pow((camFar / camNear), static_cast<float>(splitIndex + 1) / static_cast<float>(numSplits));
		float farZUni = camNear + (camFar - camNear) * (static_cast<float>(splitIndex + 1) / static_cast<float>(numSplits));
		float farZ = splitLogUni_C * farZLog + (1.0f - splitLogUni_C) * farZUni;

		return float2(nearZ, farZ);
	}

	void CascadedShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		ShadowTechnique::PrepareShadow(view);

		// Init shadow map texture
		TextureDesc depthDesc;
		depthDesc.width = _shadowMapSize.x();
		depthDesc.height = _shadowMapSize.y();
		depthDesc.depth = 1;
		depthDesc.arraySize = _numSplits;
		depthDesc.mipLevels = 1;
		depthDesc.bCube = false;
		depthDesc.cpuAccess = 0;
		depthDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_DEPTH_STENCIL;
		depthDesc.format = RENDER_FORMAT_R24G8_TYPELESS;
		depthDesc.sampleCount = 1;
		depthDesc.sampleQuality = 0;

		auto depthTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, depthDesc });
		auto depthTex = depthTexRef->Get()->Cast<Texture>();

		// Init camera
		float2 splitNearFar = float2(view->GetCamera()->GetNear(), view->GetCamera()->GetFar());
		splitNearFar.y() = std::min<float>(splitNearFar.y(), _maxDistance);
		float tanFovDiv2 = std::tan( view->GetCamera()->Cast<PerspectiveCamera>()->GetFovAngle() * 0.5f );
		float aspectRatio = view->GetCamera()->Cast<PerspectiveCamera>()->GetAspectRatio();

		auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(GetLight());
		float3 _lightUp = abs(dirLight->Direction().y()) == 1.0f ? float3(1.0f, 0.0f, 0.0f) : float3(0.0f, 1.0f, 0.0f);

		_splits.clear();
		_splitsShadowViewMin.clear();
		_splitsShadowViewMax.clear();
		for (int32_t splitIndex = 0; splitIndex < _numSplits; ++splitIndex)
		{
			auto dirCamera = std::make_shared<OrthogonalCamera>();
			dirCamera->LookTo(dirLight->GetPos(), dirLight->Direction(), _lightUp);

			const auto & dirView = dirCamera->GetViewMatrix();
			auto cameraDirPos = transform_coord(view->GetCamera()->GetPos(), dirView);
			auto cameraDirXAxis = transform_coord(view->GetCamera()->GetXAxis(), dirView);
			auto cameraDirYAxis = transform_coord(view->GetCamera()->GetYAxis(), dirView);
			auto cameraDirZAxis = transform_coord(view->GetCamera()->GetZAxis(), dirView);

			/*float3 cameraDirPos, cameraDirXAxis, cameraDirYAxis, cameraDirZAxis;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&cameraDirPos), cameraDirPosXM);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&cameraDirXAxis), cameraDirXAxisXM);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&cameraDirYAxis), cameraDirYAxisXM);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&cameraDirZAxis), cameraDirZAxisXM);*/

			auto split = ComputeSplit(splitNearFar, splitIndex, _numSplits);

			float nearZ = split.x();
			float nearY = tanFovDiv2 * nearZ;
			float nearX = nearY * aspectRatio;

			float farZ = split.y();
			float farY = tanFovDiv2 * farZ;
			float farX = farY * aspectRatio;

			float3 pn0 = cameraDirPos + cameraDirZAxis * nearZ - cameraDirXAxis * nearX + cameraDirYAxis * nearY;
			float3 pn1 = cameraDirPos + cameraDirZAxis * nearZ + cameraDirXAxis * nearX + cameraDirYAxis * nearY;
			float3 pn2 = cameraDirPos + cameraDirZAxis * nearZ + cameraDirXAxis * nearX - cameraDirYAxis * nearY;
			float3 pn3 = cameraDirPos + cameraDirZAxis * nearZ - cameraDirXAxis * nearX - cameraDirYAxis * nearY;

			float3 pf0 = cameraDirPos + cameraDirZAxis * farZ - cameraDirXAxis * farX + cameraDirYAxis * farY;
			float3 pf1 = cameraDirPos + cameraDirZAxis * farZ + cameraDirXAxis * farX + cameraDirYAxis * farY;
			float3 pf2 = cameraDirPos + cameraDirZAxis * farZ + cameraDirXAxis * farX - cameraDirYAxis * farY;
			float3 pf3 = cameraDirPos + cameraDirZAxis * farZ - cameraDirXAxis * farX - cameraDirYAxis * farY;

			float3 min = min_vec({ pn0, pn1, pn2, pn3, pf0, pf1, pf2, pf3 });
			float3 max = max_vec({ pn0, pn1, pn2, pn3, pf0, pf1, pf2, pf3 });
			max.x() += 1.0f;
			max.y() += 1.0f;
			max.z() += 50.0f;
			min.x() -= 1.0f;
			min.y() -= 1.0f;
			min.z() -= 100.0f;

			dirCamera->SetViewBox(min.x(), max.x(), min.y(), max.y(), min.z(), max.z());

			// Update cb
			_cascadedShadowView.shadowWorldToViewMatrix[splitIndex] = dirCamera->GetViewMatrix();
			_cascadedShadowView.shadowViewToClipMatrix[splitIndex] = dirCamera->GetProjMatrix();
			_cascadedShadowView.shadowWorldToClipMatrix[splitIndex] = dirCamera->GetViewProjMatrix();
			_cascadedShadowView.shadowViewNear = dirCamera->GetNear();
			_cascadedShadowView.shadowViewFar = dirCamera->GetFar();
			_cascadedShadowView.shadowViewLength = _cascadedShadowView.shadowViewFar - _cascadedShadowView.shadowViewNear;
			_cascadedShadowView.shadowViewSize = float2((float)_shadowMapSize.x(), (float)_shadowMapSize.y());
			_cascadedShadowView.shadowInvViewSize = 1.0f / _cascadedShadowView.shadowViewSize;


			// Render depth
			_depthRenderView[splitIndex]->SetViewport(GetTextureQuadViewport(depthTex));
			_depthRenderView[splitIndex]->SetCamera(dirCamera);
			_depthRenderView[splitIndex]->UpdateParamsBuffer();
			RenderDepth(depthTex->GetDepthStencilView(0, splitIndex, 1, RENDER_FORMAT_D24_UNORM_S8_UINT), _depthRenderView[splitIndex]);

			_splits.push_back(split.x());
			if (splitIndex == _numSplits - 1)
				_splits.push_back(split.y());

			_splitsShadowViewMin.push_back(min);
			_splitsShadowViewMax.push_back(max);
		}

		auto mappedData = _cascadedShadowViewCB->Map(MAP_WRITE_DISCARD);
		memcpy(mappedData.pData, &_cascadedShadowView, sizeof(_cascadedShadowView));
		_cascadedShadowViewCB->UnMap();


		_shadowMaps[view] = depthTexRef;
	}

	void CascadedShadow::PrepareRSM(const Ptr<RenderView> & view)
	{
		ShadowTechnique::PrepareRSM(view);

		// Init rsm texture
		TextureDesc texDesc;
		texDesc.width = _rsmSize.x();
		texDesc.height = _rsmSize.y();
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bCube = false;
		texDesc.cpuAccess = 0;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		auto rsmRadianceRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto rsmRadiance = rsmRadianceRef->Get()->Cast<Texture>();

		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_DEPTH_STENCIL;
		texDesc.format = RENDER_FORMAT_R32_TYPELESS;
		auto rsmDepthRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto rsmDepth = rsmDepthRef->Get()->Cast<Texture>();

		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.format = RENDER_FORMAT_R8G8B8A8_UNORM;
		auto rsmNormalRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto rsmNormal = rsmNormalRef->Get()->Cast<Texture>();

		// Init camera
		float2 splitNearFar = float2(view->GetCamera()->GetNear(), view->GetCamera()->GetFar());
		float tanFovDiv2 = std::tan(view->GetCamera()->Cast<PerspectiveCamera>()->GetFovAngle() * 0.5f);
		float aspectRatio = view->GetCamera()->Cast<PerspectiveCamera>()->GetAspectRatio();

		auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(GetLight());
		float3 _lightUp = abs(dirLight->Direction().y()) == 1.0f ? float3(1.0f, 0.0f, 0.0f) : float3(0.0f, 1.0f, 0.0f);

		for (int32_t splitIndex = 0; splitIndex < 1; ++splitIndex)
		{
			auto dirCamera = std::make_shared<OrthogonalCamera>();
			dirCamera->LookTo(dirLight->GetPos(), dirLight->Direction(), _lightUp);

			const auto & dirView = dirCamera->GetViewMatrix();

			auto sceneAABB = Global::GetRenderEngine()->GetSceneRenderObjsCuller()->GetSceneAABB();
			float3 center = sceneAABB.Center();

			float3 lightViewMin = FLT_MAX;
			float3 lightViewMax = -FLT_MAX;

			for (int i = 0; i < 8; ++i)
			{
				float3 p = center;
				p += (i & 1) ? float3(1.0f, 0.0f, 0.0f) * sceneAABB.Extents().x() : float3(1.0f, 0.0f, 0.0f) * -sceneAABB.Extents().x();
				p += (i & 2) ? float3(0.0f, 1.0f, 0.0f) * sceneAABB.Extents().y() : float3(0.0f, 1.0f, 0.0f) * -sceneAABB.Extents().y();
				p += (i & 4) ? float3(0.0f, 0.0f, 1.0f) * sceneAABB.Extents().z() : float3(0.0f, 0.0f, 1.0f) * -sceneAABB.Extents().z();

				auto p_lightView = transform_coord(p, dirView);
				/*float3 p_lightView;
				XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&p_lightView), p_lightViewXM);*/

				lightViewMin = min_vec(lightViewMin, p_lightView);
				lightViewMax = max_vec(lightViewMax, p_lightView);
			}

			dirCamera->SetViewBox(lightViewMin.x(), lightViewMax.x(), lightViewMin.y(), lightViewMax.y(), lightViewMin.z(), lightViewMax.z());

			// Update cb
			_cascadedRSMView.shadowWorldToViewMatrix[splitIndex] = dirCamera->GetViewMatrix();
			_cascadedRSMView.shadowViewToClipMatrix[splitIndex] = dirCamera->GetProjMatrix();
			_cascadedRSMView.shadowWorldToClipMatrix[splitIndex] = dirCamera->GetViewProjMatrix();
			_cascadedRSMView.shadowViewNear = dirCamera->GetNear();
			_cascadedRSMView.shadowViewFar = dirCamera->GetFar();
			_cascadedRSMView.shadowViewLength = _cascadedRSMView.shadowViewFar - _cascadedRSMView.shadowViewNear;
			_cascadedRSMView.shadowViewSize = float2((float)_shadowMapSize.x(), (float)_shadowMapSize.y());
			_cascadedRSMView.shadowInvViewSize = 1.0f / _cascadedRSMView.shadowViewSize;


			// Render depth
			_rsmRenderView[splitIndex]->SetViewport(GetTextureQuadViewport(rsmRadiance));
			_rsmRenderView[splitIndex]->SetCamera(dirCamera);
			_rsmRenderView[splitIndex]->UpdateParamsBuffer();
			RenderRSM(
				_rsmRenderView[splitIndex], 
				rsmRadiance->GetRenderTargetView(0, 0, 1), 
				rsmDepth->GetDepthStencilView(0, splitIndex, 1, RENDER_FORMAT_D32_FLOAT), 
				rsmNormal->GetRenderTargetView(0, 0, 1));

			/*texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
			texDesc.format = RENDER_FORMAT_R32_FLOAT;
			auto rsmLinearDepthRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
			auto rsmLinearDepth = rsmLinearDepthRef->Get()->Cast<Texture>();*/

			/*LinearizeDepth(
				rsmDepth->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS), 
				_rsmRenderView[splitIndex], 
				rsmLinearDepth->GetRenderTargetView(0, 0, 1));

			rsmDepthRef = rsmLinearDepthRef;
			rsmDepth = rsmLinearDepth;*/

			/*_splits.push_back(split.x);
			if (splitIndex == _numSplits - 1)
				_splits.push_back(split.y);

			_splitsShadowViewMin.push_back(min);
			_splitsShadowViewMax.push_back(max);*/
		}

		/*auto mappedData = _cascadedShadowViewCB->Map(MAP_WRITE_DISCARD);
		memcpy(mappedData.pData, &_cascadedShadowView, sizeof(_cascadedShadowView));
		_cascadedShadowViewCB->UnMap();*/


		_rsms = { rsmRadianceRef, rsmDepthRef, rsmNormalRef };
	}

	void CascadedShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		ShadowTechnique::BindMacros(view, outMacros);

		outMacros["NUM_SHADOW_VIEWS"] = std::to_string(CascadedShadowMaxSplits::VALUE);
		outMacros["CASCADED_NUM_SPLITS"] = std::to_string(_numSplits);
	}

	void CascadedShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		ShadowTechnique::BindShaderParams(shader, view);

		auto shadowMap = _shadowMaps[view]->Get()->Cast<Texture>();
		if(shadowMap->GetDesc().format == RENDER_FORMAT_R24G8_TYPELESS)
			shader->SetSRV("shadowMap", shadowMap->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
		else
			shader->SetSRV("shadowMap", shadowMap->GetShaderResourceView());
		shader->SetCB("cb_shadowView_shared", _cascadedShadowViewCB);
		shader->SetScalar("cascadedSplits", &_splits[0], (int32_t)(sizeof(_splits[0]) * _splits.size()));

		std::vector<float> cascadedShadowViewNear;
		std::vector<float> cascadedShadowViewFar;
		std::vector<float> cascadedShadowViewLength;
		for (int32_t i = 0; i < _numSplits; ++i)
		{
			float n = _depthRenderView[i]->GetCamera()->GetNear();
			float f = _depthRenderView[i]->GetCamera()->GetFar();
			cascadedShadowViewNear.push_back(n);
			cascadedShadowViewFar.push_back(f);
			cascadedShadowViewLength.push_back(f - n);
		}
		shader->SetScalar("cascadedShadowViewNear", &cascadedShadowViewNear[0], (int32_t)(sizeof(cascadedShadowViewNear[0]) * cascadedShadowViewNear.size()));
		shader->SetScalar("cascadedShadowViewFar", &cascadedShadowViewFar[0], (int32_t)(sizeof(cascadedShadowViewFar[0]) * cascadedShadowViewFar.size()));
		shader->SetScalar("cascadedShadowViewLength", &cascadedShadowViewLength[0], (int32_t)(sizeof(cascadedShadowViewLength[0]) * cascadedShadowViewLength.size()));
	}
	
	/**
	 *		PCFShadow
	 */

	void PCFShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		outMacros["SHADOW_TECH"] = "SHADOW_TECH_PCF";
	}

	void PCFShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		shader->SetScalar("pcfFilterSize", _filterSize);

		auto sampler = SamplerTemplate<
			FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
			TEXTURE_ADDRESS_CLAMP,
			TEXTURE_ADDRESS_CLAMP,
			TEXTURE_ADDRESS_CLAMP,
			0,
			COMPARISON_LESS>::Get();
		shader->SetSampler("pcfSampler", sampler);
	}


	/**
	*		SpotLightPCFShadow
	*/

	void SpotLightPCFShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		SpotLightShadow::PrepareShadow(view);

		auto depthTex = _shadowMaps[nullptr]->Get()->Cast<Texture>();

		// Linearize
		auto linearDepthDesc = depthTex->GetDesc();
		linearDepthDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		linearDepthDesc.format = RENDER_FORMAT_R32_FLOAT;
		auto linearDepthTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, linearDepthDesc });

		LinearizeDepth(
			depthTex->GetShaderResourceView(0, 1, 0, 1, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS), 
			_depthRenderView, 
			linearDepthTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1));

		_shadowMaps[nullptr] = linearDepthTexRef;
	}

	void SpotLightPCFShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		SpotLightShadow::BindMacros(view, outMacros);
		PCFShadow::BindMacros(view, outMacros);
	}

	void SpotLightPCFShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		SpotLightShadow::BindShaderParams(shader, view);
		PCFShadow::BindShaderParams(shader, view);
	}


	/**
	*		PointLightPCFShadow
	*/

	void PointLightPCFShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		PointLightShadow::PrepareShadow(view);

		auto depthTex = _shadowMaps[nullptr]->Get()->Cast<Texture>();

		// Linearize
		auto linearDepthDesc = depthTex->GetDesc();
		linearDepthDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		linearDepthDesc.format = RENDER_FORMAT_R32_FLOAT;
		auto linearDepthTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, linearDepthDesc });

		for (int i = 0; i < 6; ++i)
		{
			LinearizeDepth(
				depthTex->GetShaderResourceView(0, 1, i, 1, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS),
				_depthRenderView,
				linearDepthTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, i, 1));
		}

		_shadowMaps[nullptr] = linearDepthTexRef;
	}

	void PointLightPCFShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		PointLightShadow::BindMacros(view, outMacros);
		PCFShadow::BindMacros(view, outMacros);
	}

	void PointLightPCFShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		PointLightShadow::BindShaderParams(shader, view);
		PCFShadow::BindShaderParams(shader, view);
	}

	/**
	*		PointLightPCFShadow
	*/

	void CascadedPCFShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		CascadedShadow::PrepareShadow(view);

		//auto depthTex = _shadowMaps[view]->Get()->Cast<Texture>();

		//// Linearize
		//auto linearDepthDesc = depthTex->GetDesc();
		//linearDepthDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		//linearDepthDesc.format = RENDER_FORMAT_R32_FLOAT;
		//auto linearDepthTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, linearDepthDesc });

		//for (int i = 0; i < linearDepthDesc.arraySize; ++i)
		//{
		//	LinearizeDepth(
		//		depthTex->GetShaderResourceView(0, 1, i, 1, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS),
		//		_depthRenderView[i],
		//		linearDepthTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, i, 1));
		//}

		//_shadowMaps[view] = linearDepthTexRef;
	}

	void CascadedPCFShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		CascadedShadow::BindMacros(view, outMacros);
		PCFShadow::BindMacros(view, outMacros);
	}

	void CascadedPCFShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		CascadedShadow::BindShaderParams(shader, view);
		PCFShadow::BindShaderParams(shader, view);



		std::vector<float2> filterSizeList;
		{
			float2 filterSize = _filterSize;
			filterSize.y() /= (_splitsShadowViewMax[0].y() - _splitsShadowViewMin[0].y()) / (_splitsShadowViewMax[0].x() - _splitsShadowViewMin[0].x());
			filterSizeList.push_back(filterSize);
		}
		for (int32_t i = 1; i < _numSplits; ++i)
		{
			float2 filterSize = filterSizeList[0];
			filterSize.x() /= (_splitsShadowViewMax[i].x() - _splitsShadowViewMin[i].x()) / (_splitsShadowViewMax[0].x() - _splitsShadowViewMin[0].x());
			filterSize.y() /= (_splitsShadowViewMax[i].y() - _splitsShadowViewMin[i].y()) / (_splitsShadowViewMax[0].y() - _splitsShadowViewMin[0].y());
			filterSizeList.push_back(filterSize);
		}
		shader->SetScalar("pcfFilterSize", &filterSizeList[0], (int32_t)(sizeof(filterSizeList[0]) * filterSizeList.size()));
	}


	/**
	*		VSMShadow
	*/

	void VSMShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		outMacros["SHADOW_TECH"] = "SHADOW_TECH_VSM";
	}

	void VSMShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		shader->SetScalar("vsmBias", _vsmBias);
		shader->SetScalar("vsmReduceBlending", _vsmReduceBlending);

		auto sampler = SamplerTemplate<
			FILTER_MIN_MAG_MIP_LINEAR,
			TEXTURE_ADDRESS_CLAMP,
			TEXTURE_ADDRESS_CLAMP,
			TEXTURE_ADDRESS_CLAMP>::Get();
		shader->SetSampler("vsmSampler", sampler);
	}

	void VSMShadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		Ptr<Shader> ps;
		if(bDoLinearize)
			ps = Shader::FindOrCreate<VSMConvertWithLinearizePS>();
		else
			ps = Shader::FindOrCreate<VSMConvertPS>();

		view->BindShaderParams(ps);
		ps->SetSRV("depthTex", depth);
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ target });
	}

	void VSMShadow::FilterVSM(const Ptr<Texture> & vsm, int32_t arrayIndex, float2 filterSize)
	{
		Blur::BoxBlur(
			vsm->GetShaderResourceView(0, 1, arrayIndex, 1), 
			vsm->GetRenderTargetView(0, arrayIndex, 1), 
			_filterNumSamples, filterSize);
	}


	/**
	*		SpotLightVSMShadow
	*/

	void SpotLightVSMShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		SpotLightShadow::PrepareShadow(view);

		auto depthTex = _shadowMaps[nullptr]->Get()->Cast<Texture>();

		// Convert to VSM
		auto vsmDesc = depthTex->GetDesc();
		vsmDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		vsmDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		auto vsmRef = TexturePool::Instance().FindFree({ TEXTURE_2D, vsmDesc });

		ConvertShadowMap(
			depthTex->GetShaderResourceView(0, 1, 0, 1, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS),
			_depthRenderView,
			vsmRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 
			true);

		// Filter
		FilterVSM(vsmRef->Get()->Cast<Texture>(), 0, _filterSize);

		_shadowMaps[nullptr] = vsmRef;
	}

	void SpotLightVSMShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		SpotLightShadow::BindMacros(view, outMacros);
		VSMShadow::BindMacros(view, outMacros);
	}

	void SpotLightVSMShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		SpotLightShadow::BindShaderParams(shader, view);
		VSMShadow::BindShaderParams(shader, view);
	}

	/**
	*		PointLightVSMShadow
	*/

	PointLightVSMShadow::PointLightVSMShadow()
	{
		_shadowMapBindAsCube = true;
	}

	void PointLightVSMShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		PointLightShadow::PrepareShadow(view);

		auto depthTex = _shadowMaps[nullptr]->Get()->Cast<Texture>();

		// Convert to VSM
		auto vsmDesc = depthTex->GetDesc();
		vsmDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		vsmDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		vsmDesc.bCube = true;
		vsmDesc.arraySize = 1;
		auto vsmRef = TexturePool::Instance().FindFree({ TEXTURE_2D, vsmDesc });

		for (int32_t i = 0; i < 6; ++i)
		{
			ConvertShadowMap(
				depthTex->GetShaderResourceView(0, 1, i, 1, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS),
				_depthRenderView,
				vsmRef->Get()->Cast<Texture>()->GetRenderTargetView(0, i, 1),
				true);
		}

		// Filter
		for (int32_t i = 0; i < 6; ++i)
		{
			FilterVSM(vsmRef->Get()->Cast<Texture>(), i, _filterSize);
		}

		_shadowMaps[nullptr] = vsmRef;
	}

	void PointLightVSMShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		PointLightShadow::BindMacros(view, outMacros);
		VSMShadow::BindMacros(view, outMacros);
	}

	void PointLightVSMShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		PointLightShadow::BindShaderParams(shader, view);
		VSMShadow::BindShaderParams(shader, view);
	}

	/**
	*		CascadedVSMShadow
	*/
	void CascadedVSMShadow::PrepareShadow(const Ptr<RenderView> & view)
	{
		CascadedShadow::PrepareShadow(view);

		auto depthTex = _shadowMaps[view]->Get()->Cast<Texture>();

		// Convert to VSM
		auto vsmDesc = depthTex->GetDesc();
		vsmDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		vsmDesc.format = _vsmFormat;
		auto vsmRef = TexturePool::Instance().FindFree({ TEXTURE_2D, vsmDesc });

		for (int32_t i = 0; i < _numSplits; ++i)
		{
			ConvertShadowMap(
				depthTex->GetShaderResourceView(0, 1, i, 1, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS),
				_depthRenderView[i],
				vsmRef->Get()->Cast<Texture>()->GetRenderTargetView(0, i, 1),
				false);
		}

		// Filter
		std::vector<float2> filterSizeList;
		{
			float2 filterSize = _filterSize;
			filterSize.y() /= (_splitsShadowViewMax[0].y() - _splitsShadowViewMin[0].y()) / (_splitsShadowViewMax[0].x() - _splitsShadowViewMin[0].x());
			filterSizeList.push_back(filterSize);
		}
		for (int32_t i = 1; i < _numSplits; ++i)
		{
			float2 filterSize = filterSizeList[0];
			filterSize.x() /= (_splitsShadowViewMax[i].x() - _splitsShadowViewMin[i].x()) / (_splitsShadowViewMax[0].x() - _splitsShadowViewMin[0].x());
			filterSize.y() /= (_splitsShadowViewMax[i].y() - _splitsShadowViewMin[i].y()) / (_splitsShadowViewMax[0].y() - _splitsShadowViewMin[0].y());
			filterSizeList.push_back(filterSize);
		}

		for (int32_t i = 0; i < _numSplits; ++i)
		{

			FilterVSM(vsmRef->Get()->Cast<Texture>(), i, filterSizeList[i]);
		}

		_shadowMaps[view] = vsmRef;
	}

	void CascadedVSMShadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		CascadedShadow::BindMacros(view, outMacros);
		VSMShadow::BindMacros(view, outMacros);
	}

	void CascadedVSMShadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		CascadedShadow::BindShaderParams(shader, view);
		VSMShadow::BindShaderParams(shader, view);
	}


	/**
	*		EVSM2Shadow
	*/

	void EVSM2Shadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		outMacros["SHADOW_TECH"] = "SHADOW_TECH_EVSM2";
	}

	void EVSM2Shadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		VSMShadow::BindShaderParams(shader, view);
	}

	void EVSM2Shadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		Ptr<Shader> ps;
		if (bDoLinearize)
			ps = Shader::FindOrCreate<EVSM2ConvertWithLinearizePS>();
		else
			ps = Shader::FindOrCreate<EVSM2ConvertPS>();

		view->BindShaderParams(ps);
		ps->SetSRV("depthTex", depth);
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ target });
	}

	/**
	*		SpotLightEVSM2Shadow
	*/

	void SpotLightEVSM2Shadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		SpotLightShadow::BindMacros(view, outMacros);
		EVSM2Shadow::BindMacros(view, outMacros);
	}

	void SpotLightEVSM2Shadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		SpotLightShadow::BindShaderParams(shader, view);
		EVSM2Shadow::BindShaderParams(shader, view);
	}

	void SpotLightEVSM2Shadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		EVSM2Shadow::ConvertShadowMap(depth, view, target, bDoLinearize);
	}

	/**
	*		PointLightEVSM2Shadow
	*/

	void PointLightEVSM2Shadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		PointLightVSMShadow::BindMacros(view, outMacros);
		EVSM2Shadow::BindMacros(view, outMacros);
	}

	void PointLightEVSM2Shadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		PointLightVSMShadow::BindShaderParams(shader, view);
		EVSM2Shadow::BindShaderParams(shader, view);
	}

	void PointLightEVSM2Shadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		EVSM2Shadow::ConvertShadowMap(depth, view, target, bDoLinearize);
	}

	/**
	*		CascadedEVSM2Shadow
	*/

	void CascadedEVSM2Shadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		CascadedVSMShadow::BindMacros(view, outMacros);
		EVSM2Shadow::BindMacros(view, outMacros);
	}

	void CascadedEVSM2Shadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		CascadedVSMShadow::BindShaderParams(shader, view);
		EVSM2Shadow::BindShaderParams(shader, view);
	}

	void CascadedEVSM2Shadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		EVSM2Shadow::ConvertShadowMap(depth, view, target, bDoLinearize);
	}


	/**
	*		EVSM4Shadow
	*/

	void EVSM4Shadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		outMacros["SHADOW_TECH"] = "SHADOW_TECH_EVSM4";
	}

	void EVSM4Shadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		VSMShadow::BindShaderParams(shader, view);
	}

	void EVSM4Shadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		Ptr<Shader> ps;
		if (bDoLinearize)
			ps = Shader::FindOrCreate<EVSM4ConvertWithLinearizePS>();
		else
			ps = Shader::FindOrCreate<EVSM4ConvertPS>();

		view->BindShaderParams(ps);
		ps->SetSRV("depthTex", depth);
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ target });
	}

	/**
	*		SpotLightEVSM4Shadow
	*/

	void SpotLightEVSM4Shadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		SpotLightShadow::BindMacros(view, outMacros);
		EVSM4Shadow::BindMacros(view, outMacros);
	}

	void SpotLightEVSM4Shadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		SpotLightShadow::BindShaderParams(shader, view);
		EVSM4Shadow::BindShaderParams(shader, view);
	}

	void SpotLightEVSM4Shadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		EVSM4Shadow::ConvertShadowMap(depth, view, target, bDoLinearize);
	}

	/**
	*		PointLightEVSM4Shadow
	*/

	void PointLightEVSM4Shadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		PointLightVSMShadow::BindMacros(view, outMacros);
		EVSM4Shadow::BindMacros(view, outMacros);
	}

	void PointLightEVSM4Shadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		PointLightVSMShadow::BindShaderParams(shader, view);
		EVSM4Shadow::BindShaderParams(shader, view);
	}

	void PointLightEVSM4Shadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		EVSM4Shadow::ConvertShadowMap(depth, view, target, bDoLinearize);
	}

	/**
	*		CascadedEVSM4Shadow
	*/

	void CascadedEVSM4Shadow::BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		CascadedVSMShadow::BindMacros(view, outMacros);
		EVSM4Shadow::BindMacros(view, outMacros);
	}

	void CascadedEVSM4Shadow::BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view)
	{
		CascadedVSMShadow::BindShaderParams(shader, view);
		EVSM4Shadow::BindShaderParams(shader, view);
	}

	void CascadedEVSM4Shadow::ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize)
	{
		EVSM4Shadow::ConvertShadowMap(depth, view, target, bDoLinearize);
	}
}