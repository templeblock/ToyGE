#include "ToyGE\RenderEngine\Effects\BokehDepthOfField.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Kernel\TextureAsset.h"

namespace ToyGE
{
	BokehDepthOfField::BokehDepthOfField()
		: _bDiskBlur(false),
		_bokehIlluminanceThreshold(1.0f),
		_minBokehSize(5.0f),
		_maxBokehSize(10.0f),
		_bokehSizeScale(1.0f),
		_bokehIlluminanceScale(1.0f),

		_focalDistance(5.0f),
		_focalAreaLength(1.0f),
		_nearAreaLength(1.0f),
		_farAreaLength(1.0f),
		_maxCoC(8.0f)
	{
	}

	void BokehDepthOfField::PreTAASetup(const Ptr<RenderView> & view)
	{
		auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

		auto cocTexRef = ComputeCoC(view, sceneLinearClipDepth, sceneClipDepth);
		view->GetViewRenderContext()->SetSharedResource("CoC", cocTexRef);
	}

	void BokehDepthOfField::Render(const Ptr<RenderView> & view)
	{
		auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		//auto cocTexRef = ComputeCoC(view, sceneLinearClipDepth, sceneClipDepth);
		//auto cocTex = cocTexRef->Get()->Cast<Texture>();
		auto cocTex = view->GetViewRenderContext()->GetSharedTexture("CoC");

		auto tileMaxTexRef = TileMax(cocTex, sceneLinearClipDepth);
		auto tileMaxTex = tileMaxTexRef->Get()->Cast<Texture>();

		auto dividingTexRef = PreDividing(tileMaxTex, cocTex, sceneLinearClipDepth);
		auto dividingTex = dividingTexRef->Get()->Cast<Texture>();

		auto blurTexRef = DOFBlur(sceneTex, cocTex, tileMaxTex, dividingTex, sceneLinearClipDepth);

		/*auto blurSceneRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneTex->GetDesc() });
		auto blurScene = blurSceneRef->Get()->Cast<Texture>();
		Blur::BoxBlur(sceneTex->GetShaderResourceView(), blurScene->GetRenderTargetView(0, 0, 1), 3, 1.0f);


		PooledBufferRef bokehPointsBufferRef;
		PooledTextureRef bokehSceneTexRef;
		ComputeBokehPoints(blurScene, cocTex, bokehPointsBufferRef, bokehSceneTexRef);
		auto bokehPointsBuffer = bokehPointsBufferRef->Get()->Cast<RenderBuffer>();
		auto bokehSceneTex = bokehSceneTexRef->Get()->Cast<Texture>();

		PooledTextureRef nearLayerTexRef, farLayerTexRef;
		SplitLayers(bokehSceneTex, cocTex, nearLayerTexRef, farLayerTexRef);
		auto nearLayerTex = nearLayerTexRef->Get()->Cast<Texture>();
		auto farLayerTex = farLayerTexRef->Get()->Cast<Texture>();

		auto halfTexDesc = nearLayerTex->GetDesc();
		halfTexDesc.width /= 2;
		halfTexDesc.height /= 2;

		auto halfNearLayerTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, halfTexDesc });
		auto halfNearLayerTex = halfNearLayerTexRef->Get()->Cast<Texture>();
		Transform(nearLayerTex->GetShaderResourceView(), halfNearLayerTex->GetRenderTargetView(0, 0, 1));
		Blur::GaussBlur(halfNearLayerTex->GetShaderResourceView(), halfNearLayerTex->GetRenderTargetView(0, 0, 1), 10, 8.0f);

		auto halfFarLayerTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, halfTexDesc });
		auto halfFarLayerTex = halfFarLayerTexRef->Get()->Cast<Texture>();
		Transform(farLayerTex->GetShaderResourceView(), halfFarLayerTex->GetRenderTargetView(0, 0, 1));
		Blur::GaussBlur(halfFarLayerTex->GetShaderResourceView(), halfFarLayerTex->GetRenderTargetView(0, 0, 1), 10, 8.0f);

		auto newSceneRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneTex->GetDesc() });
		auto newScene = newSceneRef->Get()->Cast<Texture>();
		Combine(bokehSceneTex, cocTex, halfNearLayerTex, halfFarLayerTex, newScene->GetRenderTargetView(0, 0, 1));

		RenderBokeh(bokehPointsBuffer, newScene->GetRenderTargetView(0, 0, 1));*/

		//view->GetViewRenderContext()->SetSharedResource("RenderResult", newSceneRef);
		view->GetViewRenderContext()->SetSharedResource("RenderResult", blurTexRef);
	}

	PooledTextureRef BokehDepthOfField::ComputeCoC(
		const Ptr<RenderView> & view,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & depthTex)
	{
		auto texDesc = linearDepthTex->GetDesc();
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.format = RENDER_FORMAT_R16_FLOAT;

		auto cocTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto cocTex = cocTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<ComputeCoCPS>();

		view->BindShaderParams(ps);

		ps->SetScalar("focalDistance", _focalDistance);
		ps->SetScalar("focalAreaLength", _focalAreaLength);
		ps->SetScalar("nearAreaLength", _nearAreaLength);
		ps->SetScalar("farAreaLength", _farAreaLength);
		ps->SetScalar("maxCoC", _maxCoC);

		ps->SetSRV("linearDepthTex", linearDepthTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ cocTex->GetRenderTargetView(0, 0, 1) },
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			depthTex->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		return cocTexRef;

		/*_fx->VariableByName("focalDistance")->AsScalar()->SetValue(&_focalDistance);
		_fx->VariableByName("focalAreaLength")->AsScalar()->SetValue(&_focalAreaLength);
		_fx->VariableByName("nearAreaLength")->AsScalar()->SetValue(&_nearAreaLength);
		_fx->VariableByName("farAreaLength")->AsScalar()->SetValue(&_farAreaLength);
		_fx->VariableByName("maxCoC")->AsScalar()->SetValue(&_maxCoC);

		float2 camNearFar = float2(camera->Near(), camera->Far());
		_fx->VariableByName("camNearFar")->AsScalar()->SetValue(&camNearFar);

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepth->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ cocTex->CreateTextureView() }, 0);
		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTargets(0.0f);

		RenderQuad(_fx->TechniqueByName("ComputeCoC"),
			0, 0, 0, 0,
			0.0f, 0.0f, 1.0f, 1.0f,
			rawDepth->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		return cocTex;*/
	}

	void BokehDepthOfField::SplitLayers(
		const Ptr<Texture> & sceneTex,
		const Ptr<Texture> & cocTex,
		PooledTextureRef & outNearLayerTex,
		PooledTextureRef & outFarLayerTex)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto texDesc = cocTex->GetDesc();
		//texDesc.width /= 2;
		//texDesc.height /= 2;
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;

		outNearLayerTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto nearLayerTex = outNearLayerTex->Get()->Cast<Texture>();
		outFarLayerTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto farLayerTex = outFarLayerTex->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<SplitLayersPS>();

		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		ps->SetSRV("cocTex", cocTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ 
			nearLayerTex->GetRenderTargetView(0, 0, 1), 
			farLayerTex->GetRenderTargetView(0, 0, 1) });

		/*auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);*/

		//float maxBlur = 5.0f;
		////_fx->VariableByName("maxBlur")->AsScalar()->SetValue(&maxBlur);
		//
		//_fx->VariableByName("cocTex")->AsShaderResource()->SetValue(cocTex->CreateTextureView());
		//_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());

		//rc->SetRenderTargets({ nearLayerTex->CreateTextureView(), farLayerTex->CreateTextureView() }, 0);
		//rc->ClearRenderTargets(0.0f);

		//RenderQuad(_fx->TechniqueByName("SplitLayers"));

		///*rc->SetRenderInput(CommonInput::QuadInput());
		//rc->SetDepthStencil(ResourceView());

		//_fx->TechniqueByName("SplitLayers")->PassByIndex(0)->Bind();
		//rc->DrawIndexed();
		//_fx->TechniqueByName("SplitLayers")->PassByIndex(0)->UnBind();

		//rc->SetViewport(preVP);*/

		//return std::make_pair(nearLayerTex, farLayerTex);
	}

	PooledTextureRef BokehDepthOfField::TileMax(
		const Ptr<Texture> & cocTex,
		const Ptr<Texture> & sceneDepthTex)
	{
		static const int tileSize = 16;

		auto texDesc = cocTex->GetDesc();
		texDesc.format = RENDER_FORMAT_R16G16_FLOAT;
		texDesc.width = (texDesc.width + tileSize - 1) / tileSize;

		auto tileMaxX = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto tileMaxXTex = tileMaxX->Get()->Cast<Texture>();

		texDesc.height = (texDesc.height + tileSize - 1) / tileSize;
		auto tileMaxY = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto tileMaxYTex = tileMaxY->Get()->Cast<Texture>();

		{
			auto ps = Shader::FindOrCreate<BokehTileMaxXPS>();

			ps->SetScalar("texSize", cocTex->GetTexSize());
			ps->SetSRV("tileMaxInTex", cocTex->GetShaderResourceView());
			ps->SetSRV("linearDepthTex", sceneDepthTex->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->Flush();

			DrawQuad({ tileMaxXTex->GetRenderTargetView(0, 0, 1) });
		}

		{
			auto ps = Shader::FindOrCreate<BokehTileMaxYPS>();

			ps->SetScalar("texSize", tileMaxXTex->GetTexSize());
			ps->SetSRV("tileMaxInTex", tileMaxXTex->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->Flush();

			DrawQuad({ tileMaxYTex->GetRenderTargetView(0, 0, 1) });
		}

		return tileMaxY;
	}

	PooledTextureRef BokehDepthOfField::PreDividing(
		const Ptr<Texture> & tileMaxTex,
		const Ptr<Texture> & cocTex,
		const Ptr<Texture> & sceneDepthTex)
	{
		auto texDesc = cocTex->GetDesc();
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		
		auto dividingTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto dividingTex = dividingTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<PreDividingPS>();
		ps->SetSRV("tileMaxTex", tileMaxTex->GetShaderResourceView());
		ps->SetSRV("cocTex", cocTex->GetShaderResourceView());
		ps->SetSRV("linearDepthTex", sceneDepthTex->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->Flush();

		DrawQuad({ dividingTex->GetRenderTargetView(0, 0, 1) });

		return dividingTexRef;
	}

	PooledTextureRef BokehDepthOfField::DOFBlur(
		const Ptr<Texture> & inTex,
		const Ptr<Texture> & cocTex,
		const Ptr<Texture> & tileMaxTex,
		const Ptr<Texture> & dividingTex,
		const Ptr<Texture> & sceneDepthTex)
	{
		auto texDesc = inTex->GetDesc();

		auto blurTempTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto blurTempTex = blurTempTexRef->Get()->Cast<Texture>();
		auto blurTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto blurTex = blurTexRef->Get()->Cast<Texture>();

		{
			auto ps = Shader::FindOrCreate<DOFBlurPS>();
			ps->SetScalar("texSize", blurTex->GetTexSize());
			ps->SetScalar("frameCount", Global::GetInfo()->frameCount);

			ps->SetSRV("blurInTex", inTex->GetShaderResourceView());
			ps->SetSRV("linearDepthTex", sceneDepthTex->GetShaderResourceView());
			ps->SetSRV("cocTex", cocTex->GetShaderResourceView());
			ps->SetSRV("tileMaxTex", tileMaxTex->GetShaderResourceView());
			ps->SetSRV("dividingTex", dividingTex->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

			ps->Flush();

			DrawQuad({ blurTex->GetRenderTargetView(0, 0, 1) });
		}

		/*{
			auto ps = Shader::FindOrCreate<DOFBlurPS>();
			ps->SetScalar("texSize", blurTex->GetTexSize());
			ps->SetScalar("frameCount", Global::GetInfo()->frameCount + 5);

			ps->SetSRV("blurInTex", blurTempTex->GetShaderResourceView());
			ps->SetSRV("linearDepthTex", sceneDepthTex->GetShaderResourceView());
			ps->SetSRV("cocTex", cocTex->GetShaderResourceView());
			ps->SetSRV("tileMaxTex", tileMaxTex->GetShaderResourceView());
			ps->SetSRV("dividingTex", dividingTex->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

			ps->Flush();

			DrawQuad({ blurTex->GetRenderTargetView(0, 0, 1) });
		}*/

		return blurTexRef;
	}

	//Ptr<Texture> BokehDepthOfField::DownSample(const Ptr<Texture> & inputTex)
	//{
	//	auto re = Global::GetRenderEngine();
	//	auto rc = re->GetRenderContext();

	//	auto texDesc = inputTex->Desc();
	//	texDesc.width /= 2;
	//	texDesc.height /= 2;

	//	auto resultTex = re->GetRenderFactory()->GetTexturePooled(texDesc);

	//	float2 texelSize = 1.0f / float2(static_cast<float>(texDesc.width), static_cast<float>(texDesc.height));
	//	_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

	//	auto preVP = rc->GetViewport();
	//	auto vp = preVP;
	//	vp.width = static_cast<float>(texDesc.width);
	//	vp.height = static_cast<float>(texDesc.height);
	//	rc->SetViewport(vp);

	//	_fx->VariableByName("downSampleInTex")->AsShaderResource()->SetValue(inputTex->CreateTextureView());

	//	rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
	//	rc->SetRenderInput(CommonInput::QuadInput());
	//	rc->SetDepthStencil(ResourceView());

	//	_fx->TechniqueByName("DownSample")->PassByIndex(0)->Bind();
	//	rc->DrawIndexed();
	//	_fx->TechniqueByName("DownSample")->PassByIndex(0)->UnBind();

	//	rc->SetViewport(preVP);

	//	return resultTex;
	//}

	//Ptr<Texture> BokehDepthOfField::DOFDiskBlur(const Ptr<Texture> & inputTex)
	//{
	//	auto re = Global::GetRenderEngine();
	//	auto rc = re->GetRenderContext();

	//	auto resultTex = re->GetRenderFactory()->GetTexturePooled(inputTex->Desc());

	//	/*auto gaussTable = Blur::GaussTable(Blur::MaxBlurRadius());
	//	_fx->VariableByName("gaussTable")->AsScalar()->SetValue(&gaussTable[0], sizeof(float) * gaussTable.size());*/

	//	float2 texelSize = 1.0f / float2(static_cast<float>(inputTex->Desc().width), static_cast<float>(inputTex->Desc().height));
	//	_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

	//	const static float maxBlur = 15;
	//	_fx->VariableByName("maxBlur")->AsScalar()->SetValue(&maxBlur);

	//	auto preVP = rc->GetViewport();
	//	auto vp = preVP;
	//	vp.width = static_cast<float>(inputTex->Desc().width);
	//	vp.height = static_cast<float>(inputTex->Desc().height);
	//	rc->SetViewport(vp);

	//	_fx->VariableByName("blurInputTex")->AsShaderResource()->SetValue(inputTex->CreateTextureView());
	//	rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
	//	rc->SetRenderInput(CommonInput::QuadInput());
	//	_fx->TechniqueByName("UnfocusedDiskBlur")->PassByIndex(0)->Bind();
	//	rc->DrawIndexed();
	//	_fx->TechniqueByName("UnfocusedDiskBlur")->PassByIndex(0)->UnBind();

	//	rc->SetViewport(preVP);

	//	return resultTex;
	//}

	void BokehDepthOfField::ComputeBokehPoints(
		const Ptr<Texture> & sceneTex,
		const Ptr<Texture> & cocTex,
		PooledBufferRef & outBokehPointsBuffer,
		PooledTextureRef & outSceneTex)
	{
		outSceneTex = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneTex->GetDesc() });
		auto resultTex = outSceneTex->Get()->Cast<Texture>();

		RenderBufferDesc bokehPointsBufDesc;
		bokehPointsBufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_UNORDERED_ACCESS;
		bokehPointsBufDesc.elementSize = 24;
		bokehPointsBufDesc.numElements = sceneTex->GetDesc().width * sceneTex->GetDesc().height;
		bokehPointsBufDesc.cpuAccess = 0;
		bokehPointsBufDesc.bStructured = true;
		outBokehPointsBuffer = BufferPool::Instance().FindFree(bokehPointsBufDesc);
		auto bokehPointBuffer = outBokehPointsBuffer->Get()->Cast<RenderBuffer>();

		auto ps = Shader::FindOrCreate<ComputeBokehPointsPS>();

		ps->SetScalar("texSize", sceneTex->GetTexSize());
		ps->SetScalar("illumThreshold", _bokehIlluminanceThreshold);
		ps->SetScalar("minBokehSize", _minBokehSize);
		ps->SetScalar("maxBokehSize", _maxBokehSize);
		ps->SetScalar("bokehSizeScale", _bokehSizeScale);

		ps->SetUAV("bokehPointsBufferAppend", bokehPointBuffer->GetUnorderedAccessView(0, 0, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_APPEND));

		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		ps->SetSRV("cocTex", cocTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ resultTex->GetRenderTargetView(0, 0, 1) });


		////Compute Bokeh Points
		//float2 texelSize = 1.0f / float2(static_cast<float>(inputTex->Desc().width), static_cast<float>(inputTex->Desc().height));
		//_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		//_fx->VariableByName("illumThreshold")->AsScalar()->SetValue(&_bokehIlluminanceThreshold);
		//_fx->VariableByName("minBokehSize")->AsScalar()->SetValue(&_minBokehSize);
		//_fx->VariableByName("maxBokehSize")->AsScalar()->SetValue(&_maxBokehSize);
		//_fx->VariableByName("bokehSizeScale")->AsScalar()->SetValue(&_bokehSizeScale);

		//
		//_fx->VariableByName("bokehPointsBuffer")->AsUAV()->SetValue(
		//	bokehBuffer->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, bokehPointsBufDesc.numElements, BUFFER_UAV_APPEND));

		//_fx->VariableByName("computeBokehInTex")->AsShaderResource()->SetValue(inputTex->CreateTextureView());

		///*auto preVP = rc->GetViewport();
		//auto vp = preVP;
		//vp.width = static_cast<float>(resultTex->Desc().width);
		//vp.height = static_cast<float>(resultTex->Desc().height);
		//rc->SetViewport(vp);*/

		//rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);

		//RenderQuad(_fx->TechniqueByName("ComputeBokehPoints"));

		///*rc->SetRenderInput(CommonInput::QuadInput());

		//_fx->TechniqueByName("ComputeBokehPoints")->PassByIndex(0)->Bind();
		//rc->DrawIndexed();
		//_fx->TechniqueByName("ComputeBokehPoints")->PassByIndex(0)->UnBind();

		//rc->SetViewport(preVP);*/

		//return std::make_pair(bokehBuffer, resultTex);
	}

	void BokehDepthOfField::RenderBokeh(
		const Ptr<RenderBuffer> & bokehPointsBuffer,
		const Ptr<RenderTargetView> & target)
	{
		static Ptr<RenderBuffer> indirectAgsBuffer;

		if (!indirectAgsBuffer)
		{
			RenderBufferDesc indirectArgsBufDesc;
			indirectArgsBufDesc.bindFlag = BUFFER_BIND_INDIRECT_ARGS;
			indirectArgsBufDesc.elementSize = 16;
			indirectArgsBufDesc.numElements = 1;
			indirectArgsBufDesc.cpuAccess = 0;
			indirectArgsBufDesc.bStructured = false;

			uint32_t initData[] = { 0, 1, 0, 0 };

			indirectAgsBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
			indirectAgsBuffer->SetDesc(indirectArgsBufDesc);
			indirectAgsBuffer->Init(initData);
		}

		bokehPointsBuffer->CopyStructureCountTo(indirectAgsBuffer, 0, 0, bokehPointsBuffer->GetDesc().numElements, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_APPEND);

		auto vs = Shader::FindOrCreate<RenderBokehVS>();
		auto gs = Shader::FindOrCreate<RenderBokehGS>();
		auto ps = Shader::FindOrCreate<RenderBokehPS>();

		vs->SetSRV("bokehPointsBuffer", bokehPointsBuffer->GetShaderResourceView(0, 0, RENDER_FORMAT_UNKNOWN));

		auto targetTex = target->Cast<TextureRenderTargetView>()->GetResource()->Cast<Texture>();
		gs->SetScalar("texSize", targetTex->GetTexSize());
		gs->SetScalar("bokehIlluminanceScale", _bokehIlluminanceScale);

		auto bokehTex = Asset::Find<TextureAsset>("Textures/Bokeh_Circle.dds");
		if (!bokehTex->IsInit())
			bokehTex->Init();
		ps->SetSRV("bokehTex", bokehTex->GetTexture()->GetShaderResourceView());

		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		vs->Flush();
		gs->Flush();
		ps->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetViewport(GetTextureQuadViewport(targetTex));

		rc->SetRenderTargets({ target });
		rc->SetDepthStencil(nullptr);
		rc->SetDepthStencilState(DepthStencilStateTemplate<false>::Get());

		rc->SetVertexBuffer({});
		rc->SetIndexBuffer(nullptr);
		rc->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);

		rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_SRC_ALPHA, BLEND_PARAM_ONE>::Get());

		rc->DrawInstancedIndirect(indirectAgsBuffer, 0);

		rc->SetBlendState(nullptr);
		rc->ResetShader(SHADER_GS);

		//auto & texDesc = (*targets.begin())->Desc();

		/*float2 texelSize = 1.0f / float2(static_cast<float>(texDesc.width), static_cast<float>(texDesc.height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		_fx->VariableByName("bokehIlluminanceScale")->AsScalar()->SetValue(&_bokehIlluminanceScale);

		_fx->VariableByName("bokehPointsBufferForRender")->AsShaderResource()->SetValue(
			bokehPointsBuffer->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, bokehPointsBuffer->Desc().numElements));
		_fx->VariableByName("bokehTex")->AsShaderResource()->SetValue(_bokehTex->CreateTextureView(0, 0));

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);

		std::vector<ResourceView> rts;
		for (auto & i : targets)
			rts.push_back(i->CreateTextureView());

		auto ri = Global::GetRenderEngine()->GetRenderFactory()->CreateRenderInput();
		ri->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);

		rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
		rc->SetRenderInput(ri);

		_fx->TechniqueByName("RenderBokeh")->PassByIndex(0)->Bind();
		rc->DrawInstancedIndirect(indirectAgsBuffer, 0);
		_fx->TechniqueByName("RenderBokeh")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		return resultTex;*/
	}

	void BokehDepthOfField::Combine(
		const Ptr<Texture> & sceneTex,
		const Ptr<Texture> & cocTex,
		const Ptr<Texture> & nearBlurTex,
		const Ptr<Texture> & farBlurTex,
		const Ptr<RenderTargetView> & target)
	{
		auto ps = Shader::FindOrCreate<DOFCombinePS>();

		ps->SetScalar("maxCoC", _maxCoC);

		ps->SetSRV("cocTex", cocTex->GetShaderResourceView());
		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		ps->SetSRV("nearBlurTex", nearBlurTex->GetShaderResourceView());
		ps->SetSRV("farBlurTex", farBlurTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ target });

		//auto re = Global::GetRenderEngine();
		//auto rc = re->GetRenderContext();

		//_fx->VariableByName("nearBlurTex")->AsShaderResource()->SetValue(nearBlurTex[0]->CreateTextureView());
		//_fx->VariableByName("farBlurTex")->AsShaderResource()->SetValue(farBlurTex[0]->CreateTextureView());
		//_fx->VariableByName("bokehLayerTex")->AsShaderResource()->SetValue(bokehTex->CreateTextureView());
		////_fx->VariableByName("farBlurTex2")->AsShaderResource()->SetValue(farBlurTex[1]->CreateTextureView());
		////_fx->VariableByName("minMaxCoCTex")->AsShaderResource()->SetValue(minMaxCoCTex->CreateTextureView());

		////float maxBlur = 15.0f;
		//_fx->VariableByName("maxCoC")->AsScalar()->SetValue(&_maxCoC);

		//rc->SetRenderTargets({ target }, 0);
		//rc->SetRenderInput(CommonInput::QuadInput());

		//_fx->TechniqueByName("Recombine")->PassByIndex(0)->Bind();
		//rc->DrawIndexed();
		//_fx->TechniqueByName("Recombine")->PassByIndex(0)->UnBind();
	}
}