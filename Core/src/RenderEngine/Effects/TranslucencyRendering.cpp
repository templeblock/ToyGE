#include "ToyGE\RenderEngine\Effects\TranslucencyRendering.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "boost\assert.hpp"
#include "ToyGE\RenderEngine\RenderBuffer.h"

namespace ToyGE
{
	TranslucencyRendering::TranslucencyRendering()
		: _causticsMapSize(512)
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"TranslucencyRendering.xml");

		_causticsFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"Caustics.xml");

		_oitSortFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"OITSortCS.xml");

		RenderBufferDesc counterBufferDesc;
		counterBufferDesc.bindFlag = BUFFER_BIND_UNORDERED_ACCESS | BUFFER_BIND_STRUCTURED;
		counterBufferDesc.cpuAccess = 0;
		counterBufferDesc.elementSize = 4;
		counterBufferDesc.numElements = 1;
		counterBufferDesc.structedByteStride = 4;
		_oitCounterBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(counterBufferDesc, nullptr);
	}

	void TranslucencyRendering::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();
		auto factory = re->GetRenderFactory();

		auto sceneTex = sharedEnviroment->GetView()->GetRenderResult();//std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);
		auto rawDepth = sharedEnviroment->ParamByName(CommonRenderShareName::RawDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto linearDepth = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer0 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(0))->As<SharedParam<Ptr<Texture>>>()->GetValue();

		//Init Textures
		auto texDesc = sceneTex->Desc();
		texDesc.mipLevels = 0;
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_GENERATE_MIPS | TEXTURE_BIND_UNORDERED_ACCESS;

		auto lightingTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto backgroundTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		/*texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		auto backFaceInfo = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		texDesc.format = RENDER_FORMAT_D32_FLOAT;
		auto backFaceRenderDepth = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);*/

		ToyGE_ASSERT(sceneTex->CopyTo(backgroundTex, 0, 0, 0, 0, 0, 0, 0));

		//Cull And Sort Objects
		std::vector<Ptr<RenderComponent>> translucentObjs;
		for (auto & obj : sharedEnviroment->GetView()->GetRenderObjects())
		{
			if (obj->GetMaterial()->IsTranslucent())
			{
				translucentObjs.push_back(obj);
			}
		}
		std::sort(translucentObjs.begin(), translucentObjs.end(),
			[&](const Ptr<RenderComponent> & obj0, const Ptr<RenderComponent> & obj1) -> bool
		{
			auto & aabb0 = obj0->GetBoundsAABB();
			auto & aabb1 = obj1->GetBoundsAABB();
			float3 cameraPos = *(reinterpret_cast<const float3*>(&sharedEnviroment->GetView()->GetCamera()->Pos()));
			float3 lookDir = *(reinterpret_cast<const float3*>(&sharedEnviroment->GetView()->GetCamera()->Look()));
			float3 center0 = *(reinterpret_cast<float3*>(&aabb0.Center));
			float3 center1 = *(reinterpret_cast<float3*>(&aabb1.Center));
			float dist0 = dot(cameraPos - center0, lookDir);
			float dist1 = dot(cameraPos - center1, lookDir);
			return dist0 > dist1;
		});

		//Rendering Caustics
		auto & view = sharedEnviroment->GetView()->GetCamera()->ViewMatrix();
		auto viewXM = XMLoadFloat4x4(&view);
		auto invViewXM = XMMatrixInverse(&XMMatrixDeterminant(viewXM), viewXM);
		XMFLOAT4X4 invView;
		XMStoreFloat4x4(&invView, invViewXM);
		_causticsFX->VariableByName("invView")->AsScalar()->SetValue(&invView);
		auto & proj = sharedEnviroment->GetView()->GetCamera()->ProjMatrix();
		_causticsFX->VariableByName("proj")->AsScalar()->SetValue(&proj);
		float2 camNearFar(sharedEnviroment->GetView()->GetCamera()->Near(), sharedEnviroment->GetView()->GetCamera()->Far());
		_causticsFX->VariableByName("camNearFar")->AsScalar()->SetValue(&camNearFar, sizeof(camNearFar));
		_causticsFX->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepth->CreateTextureView());
		_causticsFX->VariableByName("gbuffer0")->AsShaderResource()->SetValue(gbuffer0->CreateTextureView());
		for (auto & light : sharedEnviroment->GetView()->GetRenderLights())
		{
			if (light->IsCastCaustics())
			{
				InitCausticsMap(light);
				auto recieverPosMap = _causticsRecieverPosMap[light];
				auto causticsMap = _causticsRenderMap[light];
				_causticsFX->VariableByName("recieverPosTexCube")->AsShaderResource()->SetValue(recieverPosMap->CreateTextureView_Cube(0, 1, 0, 1));
				_causticsFX->VariableByName("causticsTexCube")->AsShaderResource()->SetValue(causticsMap->CreateTextureView_Cube(0, 1, 0, 1));
				rc->SetRenderTargets({ backgroundTex->CreateTextureView() }, 0);
				rc->SetDepthStencil(ResourceView());
				rc->SetRenderInput(CommonInput::QuadInput());
				_causticsFX->TechniqueByName("CausticsRendering")->PassByIndex(0)->Bind();
				rc->DrawIndexed();
				_causticsFX->TechniqueByName("CausticsRendering")->PassByIndex(0)->UnBind();
			}
		}

		rc->SetDepthStencil(rawDepth->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		sharedEnviroment->GetView()->BindParams(_fx);

		std::vector<Ptr<RenderComponent>> oitRenderObjs;

		_fx->SetExtraMacros({});
		int32_t objIndex = 0;
		while (objIndex < static_cast<int32_t>(translucentObjs.size()))
		{
			oitRenderObjs.clear();

			while (objIndex < static_cast<int32_t>(translucentObjs.size()) && !translucentObjs[objIndex]->GetMaterial()->IsRefraction())
				oitRenderObjs.push_back(translucentObjs[objIndex++]);

			for (auto & light : sharedEnviroment->GetView()->GetRenderLights())
			{
				OITRender(
					oitRenderObjs,
					light,
					sharedEnviroment->GetView()->GetCamera(),
					sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView(),
					backgroundTex,
					rawDepth->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

				sceneTex->CopyTo(backgroundTex, 0, 0, 0, 0, 0, 0, 0);
			}
			//Embient
			OITRender(
				oitRenderObjs,
				nullptr,
				sharedEnviroment->GetView()->GetCamera(),
				sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView(),
				backgroundTex,
				rawDepth->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

			sceneTex->CopyTo(backgroundTex, 0, 0, 0, 0, 0, 0, 0);

			//Refraction
			while (objIndex < static_cast<int32_t>(translucentObjs.size()) && translucentObjs[objIndex]->GetMaterial()->IsRefraction())
			{
				RenderRefractionObject(
					translucentObjs[objIndex],
					sharedEnviroment->GetView()->GetRenderLights(),
					sharedEnviroment->GetView()->GetCamera(),
					sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView(),
					backgroundTex,
					rawDepth->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT),
					linearDepth);

				sceneTex->CopyTo(backgroundTex, 0, 0, 0, 0, 0, 0, 0);

				++objIndex;
			}
		}

		backgroundTex->Release();
		lightingTex->Release();
		resultTex->Release();
	}

	void TranslucencyRendering::OITRender(
		const std::vector<Ptr<RenderComponent>> & objs,
		const Ptr<LightComponent> & light,
		const Ptr<Camera> & camera,
		const ResourceView & target,
		const Ptr<Texture> & backgroundTex,
		const ResourceView & rawDepth)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto targetTex = std::static_pointer_cast<Texture>(target.resource);

		int32_t maxNumLayers = 5;

		TextureDesc texDesc;
		texDesc.width = targetTex->Desc().width * maxNumLayers;
		texDesc.height = targetTex->Desc().height * maxNumLayers;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_2D;
		texDesc.bindFlag = TEXTURE_BIND_UNORDERED_ACCESS | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;

		texDesc.format = RENDER_FORMAT_R32_SINT;
		auto listHeadTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		rc->ClearRenderTargets({ listHeadTex->CreateTextureView() }, -1.0f);

		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		auto listTransimittanceTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto listLigthtingTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		auto listDepthLinkTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		rc->ClearRenderTargets({ listDepthLinkTex->CreateTextureView() }, -1.0f);

		bool bInit = true;

		/*auto & cameraPos = camera->Pos();
		_fx->VariableByName("viewPos")->AsScalar()->SetValue(&cameraPos, sizeof(cameraPos));
		_fx->VariableByName("view")->AsScalar()->SetValue(&camera->ViewMatrix());
		_fx->VariableByName("proj")->AsScalar()->SetValue(&camera->ProjMatrix());
		int2 texSize = int2(targetTex->Desc().width, targetTex->Desc().height);
		_fx->VariableByName("texSize")->AsScalar()->SetValue(&texSize);*/
		int2 texSize = int2(targetTex->Desc().width, targetTex->Desc().height);
		_fx->VariableByName("texSize")->AsScalar()->SetValue(&texSize);

		_fx->VariableByName("oitCounter")->AsUAV()->SetValue(_oitCounterBuffer->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, 1, BUFFER_UAV_COUNTER, 0));
		_fx->VariableByName("listHead")->AsUAV()->SetValue(listHeadTex->CreateTextureView());
		_fx->VariableByName("listTransmittance")->AsUAV()->SetValue(listTransimittanceTex->CreateTextureView());
		_fx->VariableByName("listLighting")->AsUAV()->SetValue(listLigthtingTex->CreateTextureView());
		_fx->VariableByName("listDepthLink")->AsUAV()->SetValue(listDepthLinkTex->CreateTextureView());

		_fx->SetExtraMacros({});
		if (light)
		{
			light->BindMacros(_fx, false, camera);
			_fx->UpdateData();
			light->BindParams(_fx, false, camera);
		}
		else
		{
			_fx->AddExtraMacro("OIT_EMBIENT", "");
		}

		Ptr<Material> curMat;
		//For Each Object
		for (auto & obj : objs)
		{
			//Update Material Params
			if (curMat != obj->GetMaterial())
			{
				curMat = obj->GetMaterial();
				curMat->BindMacros(_fx);
				_fx->UpdateData();
				curMat->BindParams(_fx);
			}

			rc->SetRenderTargets({}, 0);
			rc->SetDepthStencil(rawDepth);

			Ptr<RenderTechnique> technique;
			if (curMat->IsDualFace())
				technique = _fx->TechniqueByName("OITSceneRenderingDualFace");
			else
				technique = _fx->TechniqueByName("OITSceneRendering");

			_fx->VariableByName("world")->AsScalar()->SetValue(&obj->GetTransformMatrix());
			rc->SetRenderInput(obj->GetMesh()->AcquireRender()->GetRenderInput());
			for (int32_t passIndex = 0; passIndex < technique->NumPasses(); ++passIndex)
			{
				technique->PassByIndex(passIndex)->Bind();
				rc->DrawIndexed();
				technique->PassByIndex(passIndex)->UnBind();
			}
		}


		//Sort And Merge
		_oitSortFX->VariableByName("texSize")->AsScalar()->SetValue(&texSize);

		_oitSortFX->VariableByName("resultTex")->AsUAV()->SetValue(target);
		_oitSortFX->VariableByName("backgroundTex")->AsShaderResource()->SetValue(backgroundTex->CreateTextureView());
		_oitSortFX->VariableByName("listHead")->AsShaderResource()->SetValue(listHeadTex->CreateTextureView());
		_oitSortFX->VariableByName("listTransmittance")->AsShaderResource()->SetValue(listTransimittanceTex->CreateTextureView());
		_oitSortFX->VariableByName("listLighting")->AsShaderResource()->SetValue(listLigthtingTex->CreateTextureView());
		_oitSortFX->VariableByName("listDepthLink")->AsShaderResource()->SetValue(listDepthLinkTex->CreateTextureView());

		rc->SetRenderTargets({}, 0);
		rc->SetDepthStencil(ResourceView());

		_oitSortFX->TechniqueByName("OITSort")->PassByIndex(0)->Bind();
		rc->Compute((targetTex->Desc().width + 15) / 16, (targetTex->Desc().height + 15) / 16, 1);
		_oitSortFX->TechniqueByName("OITSort")->PassByIndex(0)->UnBind();

		listHeadTex->Release();
		listTransimittanceTex->Release();
		listLigthtingTex->Release();
		listDepthLinkTex->Release();
	}

	void TranslucencyRendering::RenderRefractionObject(
		const Ptr<RenderComponent> & obj,
		const std::vector<Ptr<LightComponent>> & lights,
		const Ptr<Camera> & camera,
		const ResourceView & target,
		const Ptr<Texture> & backgroundTex,
		const ResourceView & rawDepth,
		const Ptr<Texture> & linearDepth)
	{
		auto texDesc = backgroundTex->Desc();
		texDesc.mipLevels = 0;
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_GENERATE_MIPS;

		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		auto backFaceInfo = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		texDesc.format = RENDER_FORMAT_D32_FLOAT;
		auto backFaceRenderDepth = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		/*auto & cameraPos = camera->Pos();
		_fx->VariableByName("viewPos")->AsScalar()->SetValue(&cameraPos, sizeof(cameraPos));
		_fx->VariableByName("view")->AsScalar()->SetValue(&camera->ViewMatrix());
		_fx->VariableByName("proj")->AsScalar()->SetValue(&camera->ProjMatrix());
		float4 cameraNearFar = float4(camera->Near(), camera->Far(), 0.0f, 0.0);
		_fx->VariableByName("cameraNearFar")->AsScalar()->SetValue(&cameraNearFar);*/

		_fx->SetExtraMacros({});
		auto curMat = obj->GetMaterial();

		curMat->BindMacros(_fx);
		_fx->UpdateData();
		curMat->BindParams(_fx);

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepth->CreateTextureView());

		//BackFace For Double Face Refraction
		if (curMat->IsDualFace())
		{
			rc->ClearRenderTargets({ backFaceInfo->CreateTextureView() }, 0.0f);
			rc->ClearDepthStencil(backFaceRenderDepth->CreateTextureView(), 0.0f, 0);

			_fx->VariableByName("world")->AsScalar()->SetValue(&obj->GetTransformMatrix());
			rc->SetRenderTargets({ backFaceInfo->CreateTextureView() }, 0);
			rc->SetDepthStencil(backFaceRenderDepth->CreateTextureView());
			rc->SetRenderInput(obj->GetMesh()->AcquireRender()->GetRenderInput());
			auto technique = _fx->TechniqueByName("RefractionBackFace");
			for (int32_t passIndex = 0; passIndex < technique->NumPasses(); ++passIndex)
			{
				technique->PassByIndex(passIndex)->Bind();
				rc->DrawIndexed();
				technique->PassByIndex(passIndex)->UnBind();
			}
		}


		//Refraction
		rc->SetDepthStencil(rawDepth);
		if (curMat->IsDualFace())
		{
			_fx->AddExtraMacro("DUAL_FACE", "");
			_fx->UpdateData();
			_fx->VariableByName("backFaceInfoTex")->AsShaderResource()->SetValue(backFaceInfo->CreateTextureView());
		}

		backgroundTex->GenerateMips();
		_fx->VariableByName("backgroundTex")->AsShaderResource()->SetValue(backgroundTex->CreateTextureView(0, 0));
		if (obj->GetEnvMap())
			_fx->VariableByName("envMap")->AsShaderResource()->SetValue(obj->GetEnvMap()->CreateTextureView_Cube(0, 0, 0, 1));

		rc->SetRenderTargets({ target }, 0);

		_fx->VariableByName("world")->AsScalar()->SetValue(&obj->GetTransformMatrix());
		rc->SetRenderInput(obj->GetMesh()->AcquireRender()->GetRenderInput());
		for (int32_t passIndex = 0; passIndex < _fx->TechniqueByName("Refraction")->NumPasses(); ++passIndex)
		{
			_fx->TechniqueByName("Refraction")->PassByIndex(passIndex)->Bind();
			rc->DrawIndexed();
			_fx->TechniqueByName("Refraction")->PassByIndex(passIndex)->UnBind();
		}
		_fx->VariableByName("backgroundTex")->AsShaderResource()->SetValue(ResourceView());


		//Lighting
		for (auto light : lights)
		{
			light->BindMacros(_fx, false, camera);
			_fx->UpdateData();
			light->BindParams(_fx, false, camera);

			Ptr<RenderTechnique> technique;
			technique = _fx->TechniqueByName("Lighting");
			//rc->SetRenderTargets({ lightingTex->CreateTextureView() }, 0);

			_fx->VariableByName("world")->AsScalar()->SetValue(&obj->GetTransformMatrix());
			rc->SetRenderInput(obj->GetMesh()->AcquireRender()->GetRenderInput());
			for (int32_t passIndex = 0; passIndex < technique->NumPasses(); ++passIndex)
			{
				technique->PassByIndex(passIndex)->Bind();
				rc->DrawIndexed();
				technique->PassByIndex(passIndex)->UnBind();
			}
		}

		//Lighting Embient
		_fx->VariableByName("world")->AsScalar()->SetValue(&obj->GetTransformMatrix());
		rc->SetRenderInput(obj->GetMesh()->AcquireRender()->GetRenderInput());
		_fx->TechniqueByName("LightingEmbient")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("LightingEmbient")->PassByIndex(0)->UnBind();

		backFaceInfo->Release();
		backFaceRenderDepth->Release();
	}

	void TranslucencyRendering::InitCausticsMapTextures(const Ptr<LightComponent> & light)
	{
		TextureDesc desc;
		desc.width = desc.height = _causticsMapSize;
		desc.depth = 1;
		desc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		desc.cpuAccess = 0;
		desc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		desc.mipLevels = 1;
		desc.sampleCount = 1;
		desc.sampleQuality = 0;
		switch (light->Type())
		{
		case LIGHT_POINT:
		{
			desc.type = TEXTURE_CUBE;
			desc.arraySize = 1;
		}
			break;

		default:
			break;
		}
		auto factory = Global::GetRenderEngine()->GetRenderFactory();
		_causticsRecieverPosMap[light] = factory->CreateTexture(desc);
		_causticsRenderMap[light] = factory->CreateTexture(desc);
	}

	Ptr<Mesh> TranslucencyRendering::CreateCausticsMesh(const Ptr<Mesh> & mesh)
	{
		auto causticsMesh = mesh->Copy();

		VertexDataDesc vertexDesc;

		VertexElementDesc elemDesc;
		elemDesc.name = "WEIGHT";
		elemDesc.format = RENDER_FORMAT_R32_FLOAT;
		elemDesc.index = 0;
		elemDesc.bytesOffset = 0;
		elemDesc.bytesSize = sizeof(float);
		elemDesc.instanceDataRate = 0;
		vertexDesc.elementsDesc.push_back(elemDesc);

		vertexDesc.numVertices = mesh->GetVertexData(0).numVertices;
		vertexDesc.vertexByteSize = sizeof(float);

		std::vector<float> weights(mesh->GetVertexData(0).numVertices);
		float all = 0.0f;
		int32_t numTriangles = mesh->NumIndices() / 3;
		auto posDescIndex = mesh->GetVertexData(0).FindElementDesc(StandardVertexElementName::Position());
		auto & posDesc = mesh->GetVertexData(0).elementsDesc[posDescIndex];
		for (int32_t triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex)
		{
			auto i0 = mesh->GetIndex(triangleIndex * 3 + 0);
			auto i1 = mesh->GetIndex(triangleIndex * 3 + 1);
			auto i2 = mesh->GetIndex(triangleIndex * 3 + 2);
			auto & p0 = mesh->GetVertexData(0).GetElement<float3>(i0, posDesc);
			auto & p1 = mesh->GetVertexData(0).GetElement<float3>(i1, posDesc);
			auto & p2 = mesh->GetVertexData(0).GetElement<float3>(i2, posDesc);

			float3 v0 = p1 - p0;
			float3 v1 = p2 - p0;
			float3 t = cross(v0, v1);
			float area = dot(t, t) * 0.5f;
			float areaVertex = area / 3.0f;
			weights[i0] += areaVertex;
			weights[i1] += areaVertex;
			weights[i2] += areaVertex;
			all += area * 0.5f;
		}

		/*for (auto & i : weights)
		{
			i /= all;
		}*/

		vertexDesc.pData = MakeBufferedDataShared(sizeof(float) * weights.size());
		memcpy(vertexDesc.pData.get(), &weights[0], sizeof(float) * weights.size());

		causticsMesh->AddVertexData(vertexDesc);

		causticsMesh->AcquireRender()->GetRenderInput()->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);

		return causticsMesh;
	}

	void TranslucencyRendering::InitCausticsMap(const Ptr<LightComponent> & light)
	{
		auto recieverPosMapFind = _causticsRecieverPosMap.find(light);
		if (recieverPosMapFind == _causticsRecieverPosMap.end())
			InitCausticsMapTextures(light);
		auto recieverPosMap = _causticsRecieverPosMap[light];
		auto causticsRenderMap = _causticsRenderMap[light];

		auto pointTex = Global::GetResourceManager(RESOURCE_TEXTURE)->As<TextureManager>()->AcquireResource(L"point.dds");

		TextureDesc desc;
		desc.width = desc.height = _causticsMapSize;
		desc.depth = 1;
		desc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		desc.cpuAccess = 0;
		desc.format = RENDER_FORMAT_D32_FLOAT;
		desc.arraySize = 1;
		desc.mipLevels = 1;
		desc.sampleCount = 1;
		desc.sampleQuality = 0;
		desc.type = TEXTURE_2D;

		auto causticsDepthTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(desc);

		switch (light->Type())
		{
		case LIGHT_POINT:
		{
			auto pointLight = std::static_pointer_cast<PointLightComponent>(light);
			

			//Init View Proj Matrix
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
				| RENDER_CONTEXT_STATE_VIEWPORT
				| RENDER_CONTEXT_STATE_RENDERTARGETS, stateSave);

			RenderViewport vp;
			vp.topLeftX = vp.topLeftY = 0.0f;
			vp.width = vp.height = static_cast<float>(_causticsMapSize);
			vp.minDepth = 0.0f;
			vp.maxDepth = 1.0f;
			rc->SetViewport(vp);

			PerspectiveCamera camera = PerspectiveCamera(XM_PIDIV2, 1, 0.1f, pointLight->MaxDistance());
			auto projXM = XMLoadFloat4x4(&camera.ProjMatrix());

			std::array<std::vector<Ptr<Cullable>>, 6> cullObjs;
			for (int32_t i = 0; i < 6; ++i)
			{
				//Set ViewProjMatrix
				camera.LookTo(pointLight->GetPos(), viewDir[i], upDir[i]);
				auto viewXM = XMLoadFloat4x4(&camera.ViewMatrix());
				auto viewProjXM = XMMatrixMultiply(viewXM, projXM);
				//XMFLOAT4X4 viewProj;
				XMStoreFloat4x4(&viewProjArray[i], viewProjXM);

				//Cull
				//std::vector<Ptr<Cullable>> cullObjs;
				auto renderObjsCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
				renderObjsCuller->Cull(camera.GetFrustum(), cullObjs[i]);
			}

			//Render RecieverPos
			for (int32_t i = 0; i < 6; ++i)
			{
				_causticsFX->VariableByName("viewProj")->AsScalar()->SetValue(&viewProjArray[i]);

				std::vector<Ptr<RenderComponent>> opaqueObjs;
				for (auto & obj : cullObjs[i])
				{
					auto com = std::static_pointer_cast<RenderComponent>(obj);
					if (!com->GetMaterial()->IsTranslucent())
						opaqueObjs.push_back(com);
				}
				
				rc->SetRenderTargets({ recieverPosMap->CreateTextureView(0, 1, i, 1) }, 0);
				rc->ClearRenderTargets({ 0.0f, 0.0f, 0.0f, 0.0f });
				rc->SetDepthStencil(causticsDepthTex->CreateTextureView());
				rc->ClearDepthStencil(1.0f, 0);

				//_causticsFX->VariableByName("viewProjArray")->AsScalar()->SetValue(&viewProjArray[0]);
				for (auto & obj : opaqueObjs)
				{
					_causticsFX->VariableByName("world")->AsScalar()->SetValue(&obj->GetTransformMatrix());
					rc->SetRenderInput(obj->GetMesh()->AcquireRender()->GetRenderInput());

					if (obj->GetMaterial()->IsPOM() && obj->GetMaterial()->NumTextures(MATERIAL_TEXTURE_HEIGHT) > 0)
					{
						float scale = obj->GetMaterial()->GetPOMScale();
						//_causticsFX->VariableByName("pomScale")->AsScalar()->SetValue(&scale, sizeof(scale));
						auto heightTex = obj->GetMaterial()->AcquireRender()->GetTexture(MATERIAL_TEXTURE_HEIGHT, 0);
						_causticsFX->VariableByName("heightTex")->AsShaderResource()->SetValue(heightTex->CreateTextureView());
						//_causticsFX->VariableByName("viewPos")->AsScalar()->SetValue(&pointLight->GetPos(), sizeof(float3));
						_causticsFX->TechniqueByName("RecieverPosPOM")->PassByIndex(0)->Bind();
						rc->DrawIndexed();
						_causticsFX->TechniqueByName("RecieverPosPOM")->PassByIndex(0)->UnBind();
					}
					else
					{
						_causticsFX->TechniqueByName("RecieverPos")->PassByIndex(0)->Bind();
						rc->DrawIndexed();
						_causticsFX->TechniqueByName("RecieverPos")->PassByIndex(0)->UnBind();
					}
				}
			}

			//Render CausticsMaps
			for (int32_t i = 0; i < 6; ++i)
			{
				_causticsFX->VariableByName("viewProj")->AsScalar()->SetValue(&viewProjArray[i]);

				std::vector<Ptr<RenderComponent>> casters;
				for (auto & obj : cullObjs[i])
				{
					auto renderCom = std::static_pointer_cast<RenderComponent>(obj);
					if (renderCom->GetMaterial()->IsTranslucent() && renderCom->IsCastCaustics())
						casters.push_back(renderCom);
				}

				rc->SetRenderTargets({ causticsRenderMap->CreateTextureView(0, 1, i, 1) }, 0);
				rc->ClearRenderTargets({ 0.0f, 0.0f, 0.0f, 0.0f });

				_causticsFX->VariableByName("recieverPosTexCube")->AsShaderResource()->SetValue(recieverPosMap->CreateTextureView_Cube(0, 1, 0, 1));
				auto & lightPos = pointLight->GetPos();
				_causticsFX->VariableByName("lightPos")->AsScalar()->SetValue(&lightPos, sizeof(lightPos));
				auto & radiance = pointLight->Radiance();
				_causticsFX->VariableByName("lightRadiance")->AsScalar()->SetValue(&radiance, sizeof(radiance));
				float pointSize = 0.1f;
				_causticsFX->VariableByName("pointSize")->AsScalar()->SetValue(&pointSize, sizeof(pointSize));
				_causticsFX->VariableByName("pointTex")->AsShaderResource()->SetValue(pointTex->CreateTextureView());

				for (auto & obj : casters)
				{
					auto & cauticsMesh = _casuticsMeshMap[obj->GetMesh()];
					if (cauticsMesh == nullptr)
						cauticsMesh = CreateCausticsMesh(obj->GetMesh());

					_causticsFX->VariableByName("world")->AsScalar()->SetValue(&obj->GetTransformMatrix());
					float refractIndex = obj->GetMaterial()->GetRefractionIndex();
					_causticsFX->VariableByName("refractIndex")->AsScalar()->SetValue(&refractIndex, sizeof(refractIndex));
					rc->SetRenderInput(cauticsMesh->AcquireRender()->GetRenderInput());
					_causticsFX->TechniqueByName("Splating")->PassByIndex(0)->Bind();
					rc->DrawVertices();
					_causticsFX->TechniqueByName("Splating")->PassByIndex(0)->UnBind();
				}
			}

			rc->RestoreState(stateSave);
		}
			break;

		default:
			break;
		}
	}
}
