#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderConfig.h"
#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderContext.h"

namespace ToyGE
{
	RenderView::RenderView()
	{
		_renderSharedEnv = std::make_shared<RenderSharedEnviroment>();

		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_CONSTANT;
		bufDesc.cpuAccess = CPU_ACCESS_WRITE;
		bufDesc.elementSize = sizeof(ViewParams);
		bufDesc.numElements = 1;
		bufDesc.structedByteStride = 0;
		_paramsBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(bufDesc, nullptr);
	}

	void RenderView::InitRenderTarget(int32_t width, int32_t height)
	{
		if (_renderTarget)
			_renderTarget->Release();
		if (_renderResult)
			_renderResult->Release();

		TextureDesc texDesc;
		texDesc.width = width;
		texDesc.height = height;
		texDesc.mipLevels = 0;
		texDesc.arraySize = 1;
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.cpuAccess = 0;
		texDesc.depth = 1;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_2D;

		_renderTarget = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		_renderResult = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
	}

	void RenderView::InitForRender()
	{
		_renderObjects.clear();
		auto objsCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
		std::vector<Ptr<Cullable>> objsCulled;
		objsCuller->Cull(_camera->GetFrustum(), objsCulled);
		//objsCuller->GetAllElements(objsCulled);
		for (auto & obj : objsCulled)
		{
			auto renderObj = std::static_pointer_cast<RenderComponent>(obj);
			//renderObj->UpdateTransform();
			_renderObjects.push_back(renderObj);
		}

		_renderLights.clear();
		auto lightsCuller = Global::GetRenderEngine()->GetSceneRenderLightsCuller();
		std::vector<Ptr<Cullable>> lightsCulled;
		lightsCuller->Cull(_camera->GetFrustum(), lightsCulled);
		for (auto & light : lightsCulled)
		{
			auto renderLight = std::static_pointer_cast<LightComponent>(light);
			//renderLight->UpdateTransform();
			_renderLights.push_back(renderLight);
		}

		_renderSharedEnv->SetView(shared_from_this());
		_renderSharedEnv->Clear();
		for (auto & i : _renderConfig->configMap)
			_renderSharedEnv->SetParam(i.first, std::make_shared<SharedParam<String>>(i.second));

		UpdateParamsBuffer();

		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTargets(
		{ _renderTarget->CreateTextureView(), _renderResult->CreateTextureView() },
		0.0f);
	}

	void RenderView::RenderPostProcess()
	{
		for (auto & render : _postProcessRenders)
			render->Render(_renderSharedEnv);
	}

	void RenderView::BindParams(const Ptr<RenderEffect> & effect) const
	{
		effect->VariableByName("cb_View")->AsConstantBuffer()->SetValue(_paramsBuffer);
	}

	void RenderView::UpdateParamsBuffer()
	{
		//ViewParams viewParams;

		_viewParams.view = GetCamera()->ViewMatrix();
		_viewParams.proj = GetCamera()->ProjMatrix();

		auto viewXM = XMLoadFloat4x4(&GetCamera()->ViewMatrix());
		auto projXM = XMLoadFloat4x4(&GetCamera()->ProjMatrix());
		auto viewProjXM = XMMatrixMultiply(viewXM, projXM);
		XMStoreFloat4x4(&_viewParams.viewProj, viewProjXM);

		_viewParams.preView = GetCamera()->GetViewMatrixCache();
		_viewParams.cameraPos = *(reinterpret_cast<const float3*>(&GetCamera()->Pos()));
		_viewParams.cameraNearFar = float2(GetCamera()->Near(), GetCamera()->Far());

		//auto targetTex = std::static_pointer_cast<Texture>(GetRenderTarget().resource);
		float viewWidth = static_cast<float>(GetRenderTarget()->Desc().width);
		float viewHeight = static_cast<float>(GetRenderTarget()->Desc().height);
		_viewParams.viewSize = float4(viewWidth, viewHeight, 1.0f / viewWidth, 1.0f / viewHeight);

		auto mappedData = _paramsBuffer->Map(MAP_WRITE_DISCARD);
		memcpy(mappedData.pData, &_viewParams, sizeof(ViewParams));
		_paramsBuffer->UnMap();
	}

	//void RenderView::InitRenderObjects()
	//{
	//	
	//}

	//void RenderView::InitRenderLights()
	//{
	//	
	//}

	//void RenderView::RenderStart()
	//{
	//	//_renderQueue->ClearSharedEnviroment();
	//	_renderQueue->InitSharedEnviroment();
	//	_renderQueue->SetRenderTarget(_renderTarget);
	//	_renderQueue->SetRenderObjects(_renderObjects);
	//	_renderQueue->SetRenderLights(_renderLights);
	//	_renderQueue->SetCamera(_camera);
	//	_renderQueue->SetViewport(_viewport);
	//}

	//void RenderView::RenderSpecific(uint32_t stepID)
	//{
	//	_renderQueue->RenderSpecific(stepID);
	//}

	//void RenderView::RenderExtra()
	//{
	//	_renderQueue->RenderExtra();
	//}
}