#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\PostProcessing.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Material.h"

namespace ToyGE
{
	uint32_t MatCmp::GetMatKey(const Ptr<Material> & mat) const
	{
		if (!mat)
			return 0;

		uint32_t key = 2;
		for (int i = 0; i < MaterialTextureTypeNum::NUM; ++i)
		{
			if (mat->GetTexture(static_cast<MaterialTextureType>(i)).size() > 0)
				key |= 1 << (31 - i);
		}

		return key;
	}

	bool MatCmp::operator()(const Ptr<Material> & mat0, const Ptr<Material> & mat1) const
	{
		auto key0 = GetMatKey(mat0);
		auto key1 = GetMatKey(mat1);

		if (key0 == key1)
			return mat0 < mat1;
		else
			return key0 < key1;
	}

	void PrimitiveDrawList::AddRenderComponent(const Ptr<RenderComponent> & component)
	{
		int32_t index = 0;
		/*for (auto & meshElement : component->GetMesh()->GetRenderData()->GetMeshElements())
		{*/

		auto mat = component->GetMeshElement()->GetMaterial();
		if (component->GetMaterial())
			mat = component->GetMaterial();

		//auto pair = std::make_pair(std::static_pointer_cast<TransformComponent>(component), component->GetMeshElement());
		drawBatches[mat].push_back(component);
		//}
	}

	RenderView::RenderView()
	{
		_viewRenderContext = std::make_shared<ViewRenderContext>();
		_viewRenderContext->primitiveDrawList = std::make_shared<PrimitiveDrawList>();

		_paramsBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_CONSTANT;
		bufDesc.cpuAccess = CPU_ACCESS_WRITE;
		bufDesc.elementSize = sizeof(ViewParams);
		bufDesc.numElements = 1;
		bufDesc.bStructured = false;
		_paramsBuffer->SetDesc(bufDesc);
		_paramsBuffer->Init(nullptr);
	}

	//void RenderView::PreRender(int32_t width, int32_t height)
	//{
	//	if (_renderTarget)
	//		_renderTarget->Release();
	//	if (_renderResult)
	//		_renderResult->Release();

	//	TextureDesc texDesc;
	//	texDesc.width = width;
	//	texDesc.height = height;
	//	texDesc.mipLevels = 0;
	//	texDesc.arraySize = 1;
	//	texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_UNORDERED_ACCESS;
	//	texDesc.cpuAccess = 0;
	//	texDesc.depth = 1;
	//	texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
	//	texDesc.sampleCount = 1;
	//	texDesc.sampleQuality = 0;
	//	texDesc.type = TEXTURE_2D;

	//	_renderTarget = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
	//	_renderResult = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
	//}

	void RenderView::PreRender()
	{
		auto objsCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
		std::vector<Ptr<Cullable>> objsCulled;
		objsCuller->Cull(_camera->GetFrustum(), objsCulled);
		for (auto & obj : objsCulled)
		{
			auto renderObj = std::static_pointer_cast<RenderComponent>(obj);
			_viewRenderContext->primitiveDrawList->AddRenderComponent(renderObj);
		}

		auto lightsCuller = Global::GetRenderEngine()->GetSceneRenderLightsCuller();
		std::vector<Ptr<Cullable>> lightsCulled;
		lightsCuller->Cull(_camera->GetFrustum(), lightsCulled);
		for (auto & light : lightsCulled)
		{
			auto renderLight = std::static_pointer_cast<LightComponent>(light);
			_viewRenderContext->lights.push_back(renderLight);
		}

		UpdateParamsBuffer();
	}

	void RenderView::PostRender()
	{
		if(_postProcessing)
			_postProcessing->Render(shared_from_this());

		auto renderResult = GetViewRenderContext()->GetSharedTexture("RenderResult");
		if (renderResult)
		{
			Transform(
				renderResult->GetShaderResourceView(),
				GetRenderTarget(),
				Vector<ColorWriteMask, 4>(COLOR_WRITE_R, COLOR_WRITE_G, COLOR_WRITE_B, COLOR_WRITE_A),
				0.0f,
				float4(_viewport.topLeftX, _viewport.topLeftY, _viewport.width, _viewport.height));
		}

		_viewRenderContext->primitiveDrawList->drawBatches.clear();
		_viewRenderContext->lights.clear();
		_viewRenderContext->sharedResources.clear();

		if (GetCamera())
			GetCamera()->SetViewMatrixCache(GetCamera()->GetViewMatrix());
	}

	void RenderView::BindShaderParams(const Ptr<Shader> & shader) const
	{
		shader->SetCB("cb_view_shared", _paramsBuffer);
	}

	void RenderView::UpdateParamsBuffer()
	{
		if (GetCamera())
		{
			_viewParams.worldToViewMatrix = GetCamera()->GetViewMatrix();
			_viewParams.viewToClipMatrix = GetCamera()->GetProjMatrix();

			auto viewXM = XMLoadFloat4x4(&GetCamera()->GetViewMatrix());
			auto projXM = XMLoadFloat4x4(&GetCamera()->GetProjMatrix());
			auto viewProjXM = XMMatrixMultiply(viewXM, projXM);
			XMStoreFloat4x4(&_viewParams.worldToClipMatrix, viewProjXM);

			auto invViewXM = XMMatrixInverse(&XMMatrixDeterminant(viewXM), viewXM);
			XMStoreFloat4x4(&_viewParams.viewToWorldMatrix, invViewXM);

			_viewParams.preWorldToViewMatrix = GetCamera()->GetViewMatrixCache();
			_viewParams.viewPos = *(reinterpret_cast<const float3*>(&GetCamera()->GetPos()));
			_viewParams.viewNear = GetCamera()->GetNear();
			_viewParams.viewFar = GetCamera()->GetFar();
			_viewParams.viewLength = _viewParams.viewFar - _viewParams.viewNear;
			_viewParams.viewWorldDir = *reinterpret_cast<const float3*>( &GetCamera()->GetZAxis() );
		}

		if (GetRenderTarget())
		{
			auto targetTex = GetRenderTarget()->GetResource()->Cast<Texture>();
			float viewWidth = static_cast<float>(targetTex->GetDesc().width);
			float viewHeight = static_cast<float>(targetTex->GetDesc().height);
			_viewParams.viewSize = float2(viewWidth, viewHeight);
			_viewParams.invViewSize = float2(1.0f / viewWidth, 1.0f / viewHeight);
		}

		auto mappedData = _paramsBuffer->Map(MAP_WRITE_DISCARD);
		memcpy(mappedData.pData, &_viewParams, sizeof(ViewParams));
		_paramsBuffer->UnMap();
	}
}