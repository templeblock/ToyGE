#include "ToyGE\RenderEngine\Effects\CascadedShadowDepthTechnique.h"
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
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\OctreeCuller.h"

namespace ToyGE
{
	namespace
	{
		static float2 ComputeSplit(
			const float2 & camNearFar, 
			int32_t splitIndex,
			int32_t numSplits)
		{
			float splitLogUni_C = 0;

			float camNear = camNearFar.x;
			float camFar = camNearFar.y;

			float nearZLog = camNear * pow((camFar / camNear), static_cast<float>(splitIndex) / static_cast<float>(numSplits));
			float nearZUni = camNear + (camFar - camNear) * (static_cast<float>(splitIndex) / static_cast<float>(numSplits));
			float nearZ = splitLogUni_C * nearZLog + (1.0f - splitLogUni_C) * nearZUni;

			float farZLog = camNear * pow((camFar / camNear), static_cast<float>(splitIndex + 1) / static_cast<float>(numSplits));
			float farZUni = camNear + (camFar - camNear) * (static_cast<float>(splitIndex + 1) / static_cast<float>(numSplits));
			float farZ = splitLogUni_C * farZLog + (1.0f - splitLogUni_C) * farZUni;

			return float2(nearZ, farZ);
		}

		static XMFLOAT4X4 ComputeCropMatrix(const float3 & min, const float3 & max)
		{
			float3 scale;
			float3 offset;
			scale.x = 2.0f / (max.x - min.x);
			scale.y = 2.0f / (max.y - min.y);
			scale.z = 1.0f / (max.z - min.z);
			offset.x = (max.x + min.x) * -0.5f * scale.x;
			offset.y = (max.y + min.y) * -0.5f * scale.y;
			offset.z = -min.z * scale.z;

			auto cropMatXM = XMMatrixSet
				(
					scale.x, 0.0f, 0.0f, 0.0f,
					0.0f, scale.y, 0.0f, 0.0f,
					0.0f, 0.0f, scale.z, 0.0f,
					offset.x, offset.y, offset.z, 1.0f
					);
			XMFLOAT4X4 cropMat;
			XMStoreFloat4x4(&cropMat, cropMatXM);

			return cropMat;
		}

		static XMFLOAT4X4 ComputeCropMatrixSplit(
			const float2 & split,
			float tanFovDiv2,
			float aspectRatio,
			const float3 & lightPos,
			const float3 & lightLook,
			const float3 & lightUp,
			const float3 & lightRight,
			float3 & outMin,
			float3 & outMax)
		{
			float nearZ = split.x;
			float nearY = tanFovDiv2 * nearZ;
			float nearX = nearY * aspectRatio;

			float farZ = split.y;
			float farY = tanFovDiv2 * farZ;
			float farX = farY * aspectRatio;

			float3 pn0 = lightPos + lightLook * nearZ - lightRight * nearX + lightUp * nearY;
			float3 pn1 = lightPos + lightLook * nearZ + lightRight * nearX + lightUp * nearY;
			float3 pn2 = lightPos + lightLook * nearZ + lightRight * nearX - lightUp * nearY;
			float3 pn3 = lightPos + lightLook * nearZ - lightRight * nearX - lightUp * nearY;

			float3 pf0 = lightPos + lightLook * farZ - lightRight * farX + lightUp * farY;
			float3 pf1 = lightPos + lightLook * farZ + lightRight * farX + lightUp * farY;
			float3 pf2 = lightPos + lightLook * farZ + lightRight * farX - lightUp * farY;
			float3 pf3 = lightPos + lightLook * farZ - lightRight * farX - lightUp * farY;

			float3 min = vecMin({ pn0, pn1, pn2, pn3, pf0, pf1, pf2, pf3 });
			float3 max = vecMax({ pn0, pn1, pn2, pn3, pf0, pf1, pf2, pf3 });
			max.x += 0.1f;
			max.y += 0.1f;
			max.z += 50.0f;
			min.x -= 0.1f;
			min.y -= 0.1f;
			min.z -= 100.0f;

			auto cropMat = ComputeCropMatrix(min, max);

			outMin = min;
			outMax = max;

			return cropMat;
		}

		static void CullRenderElements(
			const float3 & splitMin,
			const float3 & splitMax,
			const XMFLOAT4X4 & invLightView,
			const float3 & lightX,
			const float3 & lightY,
			const float3 & lightZ,
			std::vector<Ptr<RenderComponent>> & outCullElements)
		{
			float3 min = FLT_MAX;
			float3 max = -FLT_MAX;

			XNA::AxisAlignedBox aabbLS;
			Math::MinMaxToAxisAlignedBox(splitMin, splitMax, aabbLS);
			float3 center;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&center), XMVector3TransformCoord(XMLoadFloat3(&aabbLS.Center), XMLoadFloat4x4(&invLightView)));

			/*float3 lightX, lightY, lightZ;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&lightX), rightXM);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&lightY), upXM);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&lightZ), lookXM);*/

			for (int i = 0; i < 8; ++i)
			{
				float3 p = center;
				p += (i & 1) ? lightX * aabbLS.Extents.x : lightX * -aabbLS.Extents.x;
				p += (i & 2) ? lightY * aabbLS.Extents.y : lightY * -aabbLS.Extents.y;
				p += (i & 4) ? lightZ * aabbLS.Extents.z : lightZ * -aabbLS.Extents.z;
				min = vecMin(p, min);
				max = vecMax(p, max);
			}
			XNA::AxisAlignedBox aabb;
			Math::MinMaxToAxisAlignedBox(min, max, aabb);

			std::vector<Ptr<Cullable>> renderCull;
			auto renderCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
			renderCuller->Cull(aabb, renderCull);
			//std::vector<Ptr<RenderComponent>> renderElements;
			for (auto & elem : renderCull)
				outCullElements.push_back(std::static_pointer_cast<RenderComponent>(elem));
		}
	}

	int32_t PSSMDepthTechnique::_maxNumSplits = 4;
	int32_t PSSMDepthTechnique::_defaultNumSplits = 3;

	PSSMDepthTechnique::PSSMDepthTechnique()
		: _splitMaxDist(50.0f)
	{
		SetNumSplits(_defaultNumSplits);
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"PSSMShadowDepth.xml");
	}

	void PSSMDepthTechnique::RenderDepth(
		const Ptr<Texture> & shadowMap,
		const Ptr<LightComponent> & light,
		const Ptr<RenderSharedEnviroment> & sharedEnv,
		const std::array<Ptr<Texture>, 3> & rsm)
	{
		auto camera = sharedEnv->GetView()->GetCamera();

		float2 nearFar = GetNearFar(sharedEnv);
		nearFar.y = std::min<float>(nearFar.y, _splitMaxDist);

		auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
		XMFLOAT3 look = dirLight->Direction();
		XMVECTOR lookXM = XMLoadFloat3(&look);
		XMVECTOR upXM = abs(look.y) < 0.99f ? XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) : XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR rightXM = XMVector3Normalize(XMVector3Cross(upXM, lookXM));
		upXM = XMVector3Cross(lookXM, rightXM);

		float3 lightX, lightY, lightZ;
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&lightX), rightXM);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&lightY), upXM);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&lightZ), lookXM);

		auto lightViewMatXM = XMMatrixLookToLH(XMVectorZero(), lookXM, upXM);
		XMFLOAT4X4 lightView;
		XMStoreFloat4x4(&lightView, lightViewMatXM);
		std::vector<XMFLOAT4X4> cropMat;
		//std::vector<Ptr<SceneObject>> objs;
		auto perspecCamera = std::static_pointer_cast<PerspectiveCamera>(camera);
		float tanFovDiv2 = tan(perspecCamera->FovAngle() * 0.5f);
		float camNear = nearFar.x;
		float camFar = nearFar.y;

		float3 camLook = float3(perspecCamera->Look().x, perspecCamera->Look().y, perspecCamera->Look().z);
		float3 camUp = float3(perspecCamera->Up().x, perspecCamera->Up().y, perspecCamera->Up().z);
		float3 camRight = float3(perspecCamera->Right().x, perspecCamera->Right().y, perspecCamera->Right().z);

		XMVECTOR camLookXM = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&camLook));
		XMVECTOR camUpXM = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&camUp));
		XMVECTOR camRightXM = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&camRight));

		auto camLookXM_L = XMVector3TransformNormal(camLookXM, lightViewMatXM);
		auto camUpXM_L = XMVector3TransformNormal(camUpXM, lightViewMatXM);
		auto camRightXM_L = XMVector3TransformNormal(camRightXM, lightViewMatXM);

		float3 camLook_L;
		float3 camUp_L;
		float3 camRight_L;
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&camLook_L), camLookXM_L);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&camUp_L), camUpXM_L);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&camRight_L), camRightXM_L);

		auto camPosXM = XMLoadFloat3(&camera->Pos());
		auto camPosXM_L = XMVector3TransformCoord(camPosXM, lightViewMatXM);
		float3 camPos_L;
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&camPos_L), camPosXM_L);

		const float splitLogUni_C = 0.0f;
		for (int32_t splitIndex = 0; splitIndex < _numSplits; ++splitIndex)
		{
			/*float nearZLog = camNear * pow((camFar / camNear), static_cast<float>(splitIndex) / static_cast<float>(_numSplits));
			float nearZUni = camNear + (camFar - camNear) * (static_cast<float>(splitIndex) / static_cast<float>(_numSplits));
			float nearZ = splitLogUni_C * nearZLog + (1.0f - splitLogUni_C) * nearZUni;
			float nearY = tanFovDiv2 * nearZ;
			float nearX = nearY * perspecCamera->AspectRatio();

			float farZLog = camNear * pow((camFar / camNear), static_cast<float>(splitIndex + 1) / static_cast<float>(_numSplits));
			float farZUni = camNear + (camFar - camNear) * (static_cast<float>(splitIndex + 1) / static_cast<float>(_numSplits));
			float farZ = splitLogUni_C * farZLog + (1.0f - splitLogUni_C) * farZUni;
			float farY = tanFovDiv2 * farZ;
			float farX = farY * perspecCamera->AspectRatio();

			float3 pn0 = camPos_L + camLook_L * nearZ - camRight_L * nearX + camUp_L * nearY;
			float3 pn1 = camPos_L + camLook_L * nearZ + camRight_L * nearX + camUp_L * nearY;
			float3 pn2 = camPos_L + camLook_L * nearZ + camRight_L * nearX - camUp_L * nearY;
			float3 pn3 = camPos_L + camLook_L * nearZ - camRight_L * nearX - camUp_L * nearY;

			float3 pf0 = camPos_L + camLook_L * farZ - camRight_L * farX + camUp_L * farY;
			float3 pf1 = camPos_L + camLook_L * farZ + camRight_L * farX + camUp_L * farY;
			float3 pf2 = camPos_L + camLook_L * farZ + camRight_L * farX - camUp_L * farY;
			float3 pf3 = camPos_L + camLook_L * farZ - camRight_L * farX - camUp_L * farY;

			float3 min = vecMin({ pn0, pn1, pn2, pn3, pf0, pf1, pf2, pf3 });
			float3 max = vecMax({ pn0, pn1, pn2, pn3, pf0, pf1, pf2, pf3 });
			max.x += 0.1f;
			max.y += 0.1f;
			max.z += 50.0f;
			min.x -= 0.1f;
			min.y -= 0.1f;
			min.z -= 100.0f;

			float3 scale;
			float3 offset;
			scale.x = 2.0f / (max.x - min.x);
			scale.y = 2.0f / (max.y - min.y);
			scale.z = 1.0f / (max.z - min.z);
			offset.x = (max.x + min.x) * -0.5f * scale.x;
			offset.y = (max.y + min.y) * -0.5f * scale.y;
			offset.z = -min.z * scale.z;

			auto cropMatXM = XMMatrixSet
				(
				scale.x, 0.0f, 0.0f, 0.0f,
				0.0f, scale.y, 0.0f, 0.0f,
				0.0f, 0.0f, scale.z, 0.0f,
				offset.x, offset.y, offset.z, 1.0f
				);
			cropMat.push_back(XMFLOAT4X4());
			XMStoreFloat4x4(&cropMat[splitIndex], cropMatXM);*/

			auto split = ComputeSplit(nearFar, splitIndex, _numSplits);
			float3 min, max;
			cropMat.push_back(
				ComputeCropMatrixSplit(
					split, 
					tanFovDiv2, 
					perspecCamera->AspectRatio(), 
					camPos_L, camLook_L, camUp_L, camRight_L, 
					min, max));

			_splitConfig[splitIndex] = split.x;
			_splitConfig[splitIndex + 1] = split.y;
			_splitMinMax[splitIndex] = std::make_pair(min, max);
		}

		_lightViewMat = lightView;
		_lightCropMat = cropMat;

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		TextureDesc tempDSDesc = shadowMap->Desc();
		tempDSDesc.arraySize = 1;
		tempDSDesc.format = RENDER_FORMAT_D32_FLOAT;
		tempDSDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		auto tempDS = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(tempDSDesc);
		auto preDepthStencil = rc->GetDepthStencil();

		auto preRTs = rc->GetRenderTargets();

		auto preViewport = rc->GetViewport();
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(shadowMap->Desc().width);
		vp.height = static_cast<float>(shadowMap->Desc().height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		auto invLightViewXM = XMMatrixInverse(&XMMatrixDeterminant(lightViewMatXM), lightViewMatXM);
		auto orientationXM = XMQuaternionRotationMatrix(invLightViewXM);
		XMFLOAT4 orientation;
		XMStoreFloat4(&orientation, orientationXM);
		XMFLOAT4X4 invLightView;
		XMStoreFloat4x4(&invLightView, invLightViewXM);

		int32_t numSplits = _numSplits;
		//_fx->VariableByName("numSplits")->AsScalar()->SetValue(&numSplits, sizeof(numSplits));
		for (int32_t splitIndex = 0; splitIndex < _numSplits; ++splitIndex)
		{
			_fx->VariableByName("lightView")->AsScalar()->SetValue(&lightView);
			_fx->VariableByName("lightCrop")->AsScalar()->SetValue(&cropMat[splitIndex], sizeof(cropMat[splitIndex]));
			//auto lightMaxDistVar = _fx->VariableByName("lightMaxDist")->AsScalar();
			//float dist = _splitMinMax[splitIndex].second.z - _splitMinMax[splitIndex].first.z;
			////for (size_t i = 0; i < _splitMinMax.size(); ++i)
			////{
			//	//lightMaxDistVar->SetValue(&dist, sizeof(dist), sizeof(float4) * i);
			//lightMaxDistVar->SetValue(&dist, sizeof(dist));
			//}
			rc->SetDepthStencil(tempDS->CreateTextureView());
			rc->ClearDepthStencil(1.0f, 0);

			rc->SetRenderTargets({ shadowMap->CreateTextureView(0, 1, splitIndex, 1) }, 0);

			

			//XNA::OrientedBox obb;
			//auto minLS = _splitMinMax[splitIndex].first;
			//auto maxLS = _splitMinMax[splitIndex].second;
			//auto minLSXM = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&minLS));
			//auto maxLSXM = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&maxLS));
			//auto minWSXM = XMVector3TransformCoord(minLSXM, invLightViewXM);
			//auto maxWSXM = XMVector3TransformCoord(maxLSXM, invLightViewXM);
			//float3 minLSWS;
			//float3 maxLSWS;
			//XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&minLSWS), minWSXM);
			//XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&maxLSWS), maxWSXM);
			//XNA::AxisAlignedBox aabb;
			//Math::MinMaxToAxisAlignedBox(minLS, maxLS, aabb);
			////obb.Center = aabb.Center;
			//XMStoreFloat3(&obb.Center, XMVector3TransformCoord(XMLoadFloat3(&aabb.Center), invLightViewXM));
			//obb.Extents = aabb.Extents;
			//obb.Orientation = orientation;
			////obb.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);

			//auto axm = XMVector3TransformNormal(XMVectorSet(-obb.Extents.x, -obb.Extents.y, obb.Extents.z, 1.0f), invLightViewXM);
			//XMFLOAT3 a;
			//XMStoreFloat3(&a, axm);

			//auto bxm = XMVector3TransformNormal(XMVectorSet(-obb.Extents.x, -obb.Extents.y, -obb.Extents.z, 1.0f), invLightViewXM);
			//XMFLOAT3 b;
			//XMStoreFloat3(&b, bxm);

			//if (_b)
			//{
			//	_b = false;

			//	Ptr<Material> mat = std::make_shared<Material>();
			//	mat->SetBaseColor(XMFLOAT3(1.0f, 1.0f, 1.0f));
			//	mat->SetRoughness(0.0f);
			//	mat->SetMetallic(0.5f);
			//	mat->SetTranslucent(false);
			//	mat->SetOpacity(0.5f);
			//	mat->SetRefraction(true);
			//	mat->SetRefractionIndex(1.5f);
			//	mat->SetDualFaceRefraction(true);

			//	auto scene = Global::GetScene();

			//	auto cube = CommonMesh::CreateCube(2.0f, 2.0f, 2.0f);
			//	auto cubeObj = cube->AddInstanceToScene(scene);
			//	cubeObj->SetMaterial(mat);
			//	cubeObj->SetScale(obb.Extents);
			//	cubeObj->SetPos(obb.Center);
			//	cubeObj->SetOrientation(obb.Orientation);
			//	cubeObj->UpdateTransform();
			//}

			std::vector<Ptr<RenderComponent>> renderElements;

			CullRenderElements(
				_splitMinMax[splitIndex].first, _splitMinMax[splitIndex].second,
				invLightView,
				lightX, lightY, lightZ,
				renderElements);

			//auto technique = _fx->TechniqueByName("PSSM");
			for (auto & elem : renderElements)
			{
				Ptr<RenderTechnique> technique;
				if (elem->GetMaterial()->NumTextures(MATERIAL_TEXTURE_OPACITYMASK) > 0)
				{
					//technique = _fx->TechniqueByName("PSSM_OPACITYTEX");
					_fx->SetExtraMacros({ { "MAT_OPACITY_TEX", "" } });
					auto opacityTex = elem->GetMaterial()->AcquireRender()->GetTexture(MATERIAL_TEXTURE_OPACITYMASK, 0);
					_fx->VariableByName("opacityTex")->AsShaderResource()->SetValue(opacityTex->CreateTextureView());
				}
				else
				{
					_fx->SetExtraMacros({});
				}
				technique = _fx->TechniqueByName("PSSM");

				//obj->Render(fx, fx->TechniqueByName(techniqueName));
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


		//Render RSM for LPV
		if (light->IsCastLPV())
		{
			tempDS->Release();
			tempDSDesc.width = rsm[0]->Desc().width;
			tempDSDesc.height = rsm[0]->Desc().height;
			tempDS = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(tempDSDesc);

			vp.width = static_cast<float>(tempDSDesc.width);
			vp.height = static_cast<float>(tempDSDesc.height);
			rc->SetViewport(vp);

			auto sceneAABB = Global::GetRenderEngine()->GetSceneRenderObjsCuller()->GetSceneAABB();

			float3 sceneMin, sceneMax;
			Math::AxisAlignedBoxToMinMax(sceneAABB, sceneMin, sceneMax);
			float3 sceneCenter = (sceneMin + sceneMax) * 0.5f;
			float3 sceneExtents = (sceneMax - sceneMin) * 0.5f;

			float3 lightMin = FLT_MAX;
			float3 lightMax = -FLT_MAX;
			for (int i = 0; i < 8; ++i)
			{
				float3 p = sceneCenter;
				p += (i & 1) ? float3(1.0f, 0.0f, 0.0f) * sceneExtents.x : float3(1.0f, 0.0f, 0.0f) * -sceneExtents.x;
				p += (i & 2) ? float3(0.0f, 1.0f, 0.0f) * sceneExtents.y : float3(0.0f, 1.0f, 0.0f) * -sceneExtents.y;
				p += (i & 4) ? float3(0.0f, 0.0f, 1.0f) * sceneExtents.z : float3(0.0f, 0.0f, 1.0f) * -sceneExtents.z;

				float3 lightP;
				XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&lightP),
					XMVector3TransformCoord(XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&p)), XMLoadFloat4x4(&lightView)));

				lightMin = vecMin(lightMin, lightP);
				lightMax = vecMax(lightMax, lightP);
			}

			auto lpvCrop = ComputeCropMatrix(lightMin, lightMax);

			_fx->VariableByName("lightView")->AsScalar()->SetValue(&lightView);
			_fx->VariableByName("lightCrop")->AsScalar()->SetValue(&lpvCrop, sizeof(lpvCrop));

			auto lpvCropXM = XMLoadFloat4x4(&lpvCrop);
			auto worldToCropXM = XMMatrixMultiply(lightViewMatXM, lpvCropXM);
			auto cropToWorldXM = XMMatrixInverse(&XMMatrixDeterminant(worldToCropXM), worldToCropXM);
			XMStoreFloat4x4(&_rsmCropToWorldMat, cropToWorldXM);

			/*auto lightMaxDistVar = _fx->VariableByName("lightMaxDist")->AsScalar();
			float dist = lightMax.z - lightMin.z;
			lightMaxDistVar->SetValue(&dist, sizeof(dist));*/
			
			rc->SetDepthStencil(tempDS->CreateTextureView());
			rc->ClearDepthStencil(1.0f, 0);

			rc->SetRenderTargets(
			{	rsm[0]->CreateTextureView(),
				rsm[1]->CreateTextureView(),
				rsm[2]->CreateTextureView() }, 0);

			//Cull Objs
			std::vector<Ptr<RenderComponent>> renderElements;

			CullRenderElements(
				lightMin, lightMax,
				invLightView,
				lightX, lightY, lightZ,
				renderElements);

			//Sort Objects
			std::vector<Ptr<RenderComponent>> sortedRenderObjs = renderElements;
			std::sort(sortedRenderObjs.begin(), sortedRenderObjs.end(),
				[&](const Ptr<RenderComponent> & obj0, const Ptr<RenderComponent> & obj1) -> bool
			{
				return (obj0->GetMaterial()->GetTypeFlags()) < (obj0->GetMaterial()->GetTypeFlags());
			});
			std::stable_sort(sortedRenderObjs.begin(), sortedRenderObjs.end(),
				[](const Ptr<RenderComponent> & obj0, const Ptr<RenderComponent> & obj1) -> bool
			{
				return obj0->GetMaterial() < obj0->GetMaterial();
			});

			//Render Each Obj
			Ptr<Material> curMat;
			for (auto & elem : sortedRenderObjs)
			{
				if (curMat != elem->GetMaterial())
				{
					_fx->SetExtraMacros({ { "RENDER_RSM", "" } });

					curMat = elem->GetMaterial();
					curMat->BindMacros(_fx);

					light->BindMacros(_fx, true, nullptr);

					_fx->UpdateData();
					curMat->BindParams(_fx);
					light->BindParams(_fx, true, nullptr);
				}

				//if (elem->GetMaterial()->NumTextures(MATERIAL_TEXTURE_OPACITYMASK) > 0)
				//{
				//	_fx->SetExtraMacros({ { "OPACITY_TEX", "" }, {"RENDER_RSM", ""} });
				//	auto opacityTex = elem->GetMaterial()->AcquireRender()->GetTexture(MATERIAL_TEXTURE_OPACITYMASK, 0);
				//	_fx->VariableByName("opacityTex")->AsShaderResource()->SetValue(opacityTex->CreateTextureView());
				//}
				//else
				//{
				//	_fx->SetExtraMacros({ { "RENDER_RSM", "" } });
				//}

				auto technique = _fx->TechniqueByName("PSSM");

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


		rc->SetDepthStencil(preDepthStencil);
		rc->SetRenderTargets(preRTs, 0);
		rc->SetViewport(preViewport);

		tempDS->Release();
	}

	void PSSMDepthTechnique::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light)
	{
		fx->VariableByName("pssmView")->AsScalar()->SetValue(&_lightViewMat);

		std::vector<XMFLOAT4X4> lightViewCrop(_lightCropMat.size());
		auto lightViewXM = XMLoadFloat4x4(&_lightViewMat);
		for (size_t splitIndex = 0; splitIndex < _lightCropMat.size(); ++splitIndex)
		{
			auto lightCropXM = XMLoadFloat4x4(&_lightCropMat[splitIndex]);
			auto lightViewCropXM = XMMatrixMultiply(lightViewXM, lightCropXM);
			XMStoreFloat4x4(&lightViewCrop[splitIndex], lightViewCropXM);
		}
		fx->VariableByName("pssmViewCrop")->AsScalar()->SetValue(&lightViewCrop[0], sizeof(lightViewCrop[0]) * lightViewCrop.size());

		auto splitConfigVar = fx->VariableByName("pssmSplitConfig")->AsScalar();
		for (size_t i = 0; i < _splitConfig.size(); ++i)
		{
			float sp = _splitConfig[i];
			splitConfigVar->SetValue(&sp, sizeof(sp), sizeof(float4) * i);
		}

		auto splitMinMaxVar = fx->VariableByName("pssmSplitMinMax")->AsScalar();
		for (size_t i = 0; i < _splitMinMax.size(); ++i)
		{
			float4 minMax = float4(_splitMinMax[i].first.x, _splitMinMax[i].first.y, _splitMinMax[i].second.x, _splitMinMax[i].second.y);
			splitMinMaxVar->SetValue(&minMax, sizeof(minMax), sizeof(float4) * i);
		}

		auto lightMaxDistVar = fx->VariableByName("pssmLightMaxDist")->AsScalar();
		for (size_t i = 0; i < _splitMinMax.size(); ++i)
		{
			float dist = _splitMinMax[i].second.z - _splitMinMax[i].first.z;
			lightMaxDistVar->SetValue(&dist, sizeof(dist), sizeof(float4) * i);
		}

		int32_t numSplits = NumSplits();
		fx->VariableByName("pssmNumSplits")->AsScalar()->SetValue(&numSplits, sizeof(numSplits));
	}

	void PSSMDepthTechnique::BindRSMParams(const Ptr<RenderEffect> & fx)
	{
		fx->VariableByName("rsmCropToWorld")->AsScalar()->SetValue(&_rsmCropToWorldMat);
	}

	void PSSMDepthTechnique::SetNumSplits(int32_t numSplits)
	{
		_numSplits = std::max<int32_t>(0, std::min<int32_t>(_maxNumSplits, numSplits));
		_splitConfig.resize(_numSplits + 1);
		_lightCropMat.resize(_numSplits);
		_splitMinMax.resize(_numSplits);
	}

	float2 PSSMDepthTechnique::GetNearFar(const Ptr<RenderSharedEnviroment> & sharedEnviroment) const
	{
		float2 nearFar;
		nearFar.x = sharedEnviroment->GetView()->GetCamera()->Near();
		nearFar.y = sharedEnviroment->GetView()->GetCamera()->Far();
		return nearFar;
	}


	SDSMDepthTechnique::SDSMDepthTechnique()
	{
		_reduceMinMaxFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"ReduceMinMax.xml");

		_reduceBlockSize = std::stoi(_reduceMinMaxFX->MacroByName("BLOCK_SIZE").value);
	}

	float2 SDSMDepthTechnique::GetNearFar(const Ptr<RenderSharedEnviroment> & sharedEnviroment) const
	{
		auto depthLinearTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		float2 nearFarH = ReduceMinMax(depthLinearTex);

		auto camera = sharedEnviroment->GetView()->GetCamera();
		float n = camera->Near();
		float f = camera->Far();

		float2 nearFar = n + (f - n) * nearFarH;
		nearFar.x = n;
		nearFar.y += 1.0f;

		return nearFar;
	}

	float2 SDSMDepthTechnique::ReduceMinMax(const Ptr<Texture> & linearDepthTex) const
	{
		auto & texDesc = linearDepthTex->Desc();
		int32_t w = static_cast<int32_t>(ceilf(static_cast<float>(texDesc.width) / static_cast<float>(_reduceBlockSize)));
		int32_t h = static_cast<int32_t>(ceilf(static_cast<float>(texDesc.height) / static_cast<float>(_reduceBlockSize)));

		TextureDesc reduceTexDesc = texDesc;
		reduceTexDesc.width = w;
		reduceTexDesc.height = h;
		reduceTexDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		reduceTexDesc.format = RENDER_FORMAT_R32G32_FLOAT;

		auto tex0 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(reduceTexDesc);
		auto tex1 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(reduceTexDesc);
		auto inTex = linearDepthTex;
		auto outTex = tex0;

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto quadInput = CommonInput::QuadInput();
		auto preInput = rc->GetRenderInput();
		rc->SetRenderInput(quadInput);

		auto preDS = rc->GetDepthStencil();
		rc->SetDepthStencil(ResourceView());
		auto preRts = rc->GetRenderTargets();

		auto preViewport = rc->GetViewport();
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		int2 reduceSize = int2(linearDepthTex->Desc().width, linearDepthTex->Desc().height);

		//Reduce Initial
		_reduceMinMaxFX->VariableByName("reduceSize")->AsScalar()->SetValue(&reduceSize);
		_reduceMinMaxFX->VariableByName("inTex")->AsShaderResource()->SetValue(inTex->CreateTextureView());
		rc->SetRenderTargets({ outTex->CreateTextureView() }, 0);
		vp.width = static_cast<float>(w);
		vp.height = static_cast<float>(h);
		rc->SetViewport(vp);
		_reduceMinMaxFX->TechniqueByName("ReduceMinMax_Initial")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_reduceMinMaxFX->TechniqueByName("ReduceMinMax_Initial")->PassByIndex(0)->UnBind();

		//Reduce loop
		inTex = tex0;
		outTex = tex1;
		while (w > 1 || h > 1)
		{
			reduceSize = int2(w, h);

			w = static_cast<int>(ceilf(static_cast<float>(w) / static_cast<float>(_reduceBlockSize)));
			h = static_cast<int>(ceilf(static_cast<float>(h) / static_cast<float>(_reduceBlockSize)));

			_reduceMinMaxFX->VariableByName("reduceSize")->AsScalar()->SetValue(&reduceSize);
			_reduceMinMaxFX->VariableByName("inTex")->AsShaderResource()->SetValue(inTex->CreateTextureView());
			rc->SetRenderTargets({ outTex->CreateTextureView() }, 0);
			vp.width = static_cast<float>(w);
			vp.height = static_cast<float>(h);
			rc->SetViewport(vp);
			_reduceMinMaxFX->TechniqueByName("ReduceMinMax")->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			_reduceMinMaxFX->TechniqueByName("ReduceMinMax")->PassByIndex(0)->UnBind();

			inTex.swap(outTex);
		}

		rc->SetRenderInput(preInput);
		rc->SetRenderTargets(preRts, 0);
		rc->SetDepthStencil(preDS);
		rc->SetViewport(preViewport);

		TextureDesc resultTexDesc = reduceTexDesc;
		resultTexDesc.width = 1;
		resultTexDesc.height = 1;
		resultTexDesc.bindFlag = 0;
		resultTexDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		resultTexDesc.cpuAccess = CPU_ACCESS_READ;

		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(resultTexDesc);

		auto copyBox = std::make_shared<Box>();
		copyBox->left = 0;
		copyBox->right = 1;
		copyBox->top = 0;
		copyBox->bottom = 1;
		copyBox->front = 0;
		copyBox->back = 1;
		inTex->CopyTo(resultTex, 0, 0, 0, 0, 0, 0, 0, copyBox);

		float2 result = *reinterpret_cast<float2*>(resultTex->Map(MAP_READ, 0, 0).pData);
		resultTex->UnMap();

		tex0->Release();
		tex1->Release();
		resultTex->Release();

		return result;
	}
}