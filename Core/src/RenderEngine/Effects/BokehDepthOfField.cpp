#include "ToyGE\RenderEngine\Effects\BokehDepthOfField.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"

namespace ToyGE
{
	BokehDepthOfField::BokehDepthOfField()
		: _bDiskBlur(false),
		_bokehIlluminanceThreshold(1.0f),
		_minBokehSize(5.0f),
		_maxBokehSize(10.0f),
		_bokehSizeScale(1.0f),
		_bokehIlluminanceScale(1.0f),

		_focalDistance(1.0f),
		_focalAreaLength(1.0f),
		_nearAreaLength(1.0f),
		_farAreaLength(1.0f),
		_maxCoC(20.0f)
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"BokehDOF.xml");
		_bokehTex = Global::GetResourceManager(RESOURCE_TEXTURE)->As<TextureManager>()->AcquireResource(L"Bokeh_Circle.dds");
	}

	void BokehDepthOfField::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);
		//auto sceneTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(targetTex->Desc());
		//targetTex->CopyTo(sceneTex, 0, 0, 0, 0, 0, 0, 0);
		auto linearDepth = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto rawDepth = sharedEnviroment->ParamByName(CommonRenderShareName::RawDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		auto cocTex = ComputeCoC(linearDepth, rawDepth, sharedEnviroment->GetView()->GetCamera());

		auto bokehPointsResult = ComputeBokehPoints(sharedEnviroment->GetView()->GetRenderResult());

		auto layers = SplitLayers(cocTex, bokehPointsResult.second);
		auto nearLayer = layers.first;
		auto farLayer = layers.second;


		auto halfNear = DownSample(nearLayer);
		auto texDesc = halfNear->Desc();
		auto nearBlur = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		Blur::GaussBlur(halfNear, 0, 0, nearBlur, 0, 0, 15, 1.0f);
		//auto nearBlur = Blur(15).GaussBlur(halfNear, 1, 0, 0, 1);

		auto halfFar = DownSample(farLayer);
		auto farBlur = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		Blur::GaussBlur(halfFar, 0, 0, farBlur, 0, 0, 15, 1.0f);
		//auto farBlur = Blur(15).GaussBlur(halfFar, 1, 0, 0, 1);

		auto bokehTex = RenderBokeh({ nearBlur, farBlur }, bokehPointsResult.first);

		Recombine({ nearBlur }, { farBlur }, bokehTex, sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView());

		sharedEnviroment->GetView()->FlipRenderTarget();
		
		//sceneTex->Release();
		cocTex->Release();
		nearLayer->Release();
		farLayer->Release();
		halfNear->Release();
		nearBlur->Release();
		halfFar->Release();
		farBlur->Release();
		bokehTex->Release();
		bokehPointsResult.second->Release();
	}

	Ptr<Texture> BokehDepthOfField::ComputeCoCPhy(const Ptr<Texture> & linearDepth, const Ptr<Texture> & rawDepth, const Ptr<Camera> & camera)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto texDesc = linearDepth->Desc();
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.format = RENDER_FORMAT_R16_FLOAT;
		
		auto cocTex = re->GetRenderFactory()->GetTexturePooled(texDesc);

		rc->ClearRenderTargets({ cocTex->CreateTextureView() }, 0.0f);

		auto phyCam = std::static_pointer_cast<PhysicalCamera>(camera);

		_fx->VariableByName("phyFocalLength")->AsScalar()->SetValue(&phyCam->GetFocalLength());
		_fx->VariableByName("phyFocalDistance")->AsScalar()->SetValue(&phyCam->GetFocalDistance());
		float apertureSize = phyCam->GetFocalLength() / phyCam->GetFStops() * 0.5f;
		_fx->VariableByName("aperture")->AsScalar()->SetValue(&apertureSize);

		//float filmDistance = _focalDistance * _focalLength / (_focalDistance - _focalLength);
		//float filmSize = std::tan(phyCam->FovAngle() * 0.5f) * filmDistance * 2.0f;
		float pixelsPerMM = static_cast<float>(texDesc.height) / phyCam->GetFilmSize().y;
		_fx->VariableByName("pixelsPerMM")->AsScalar()->SetValue(&pixelsPerMM);

		float2 camNearFar = float2(camera->Near(), camera->Far());
		_fx->VariableByName("camNearFar")->AsScalar()->SetValue(&camNearFar);

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepth->CreateTextureView());

		rc->SetRenderTargets({ cocTex->CreateTextureView() }, 0);
		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(rawDepth->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		_fx->TechniqueByName("ComputeCoCPhy")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("ComputeCoCPhy")->PassByIndex(0)->UnBind();

		return cocTex;
	}

	Ptr<Texture> BokehDepthOfField::ComputeCoC(const Ptr<Texture> & linearDepth, const Ptr<Texture> & rawDepth, const Ptr<Camera> & camera)
	{
		auto texDesc = linearDepth->Desc();
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.format = RENDER_FORMAT_R16_FLOAT;

		auto cocTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		_fx->VariableByName("focalDistance")->AsScalar()->SetValue(&_focalDistance);
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

		return cocTex;
	}

	std::pair<Ptr<Texture>, Ptr<Texture>> BokehDepthOfField::SplitLayers(const Ptr<Texture> & cocTex, const Ptr<Texture> & sceneTex)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto texDesc = cocTex->Desc();
		//texDesc.width /= 2;
		//texDesc.height /= 2;
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;

		auto nearLayerTex = re->GetRenderFactory()->GetTexturePooled(texDesc);
		auto farLayerTex = re->GetRenderFactory()->GetTexturePooled(texDesc);

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);

		float maxBlur = 5.0f;
		//_fx->VariableByName("maxBlur")->AsScalar()->SetValue(&maxBlur);
		
		_fx->VariableByName("cocTex")->AsShaderResource()->SetValue(cocTex->CreateTextureView());
		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());

		rc->SetRenderTargets({ nearLayerTex->CreateTextureView(), farLayerTex->CreateTextureView() }, 0);
		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		_fx->TechniqueByName("SplitLayers")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("SplitLayers")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		return std::make_pair(nearLayerTex, farLayerTex);
	}

	Ptr<Texture> BokehDepthOfField::DownSample(const Ptr<Texture> & inputTex)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto texDesc = inputTex->Desc();
		texDesc.width /= 2;
		texDesc.height /= 2;

		auto resultTex = re->GetRenderFactory()->GetTexturePooled(texDesc);

		float2 texelSize = 1.0f / float2(static_cast<float>(texDesc.width), static_cast<float>(texDesc.height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);

		_fx->VariableByName("downSampleInTex")->AsShaderResource()->SetValue(inputTex->CreateTextureView());

		rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		_fx->TechniqueByName("DownSample")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("DownSample")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		return resultTex;
	}

	Ptr<Texture> BokehDepthOfField::DOFDiskBlur(const Ptr<Texture> & inputTex)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto resultTex = re->GetRenderFactory()->GetTexturePooled(inputTex->Desc());

		/*auto gaussTable = Blur::GaussTable(Blur::MaxBlurRadius());
		_fx->VariableByName("gaussTable")->AsScalar()->SetValue(&gaussTable[0], sizeof(float) * gaussTable.size());*/

		float2 texelSize = 1.0f / float2(static_cast<float>(inputTex->Desc().width), static_cast<float>(inputTex->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		const static float maxBlur = 15;
		_fx->VariableByName("maxBlur")->AsScalar()->SetValue(&maxBlur);

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(inputTex->Desc().width);
		vp.height = static_cast<float>(inputTex->Desc().height);
		rc->SetViewport(vp);

		_fx->VariableByName("blurInputTex")->AsShaderResource()->SetValue(inputTex->CreateTextureView());
		rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
		rc->SetRenderInput(CommonInput::QuadInput());
		_fx->TechniqueByName("UnfocusedDiskBlur")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("UnfocusedDiskBlur")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		return resultTex;
	}

	std::pair<Ptr<RenderBuffer>, Ptr<Texture>> BokehDepthOfField::ComputeBokehPoints(const Ptr<Texture> & inputTex)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto resultTex = re->GetRenderFactory()->GetTexturePooled(inputTex->Desc());

		//Compute Bokeh Points
		float2 texelSize = 1.0f / float2(static_cast<float>(inputTex->Desc().width), static_cast<float>(inputTex->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		_fx->VariableByName("illumThreshold")->AsScalar()->SetValue(&_bokehIlluminanceThreshold);
		_fx->VariableByName("minBokehSize")->AsScalar()->SetValue(&_minBokehSize);
		_fx->VariableByName("maxBokehSize")->AsScalar()->SetValue(&_maxBokehSize);
		_fx->VariableByName("bokehSizeScale")->AsScalar()->SetValue(&_bokehSizeScale);

		RenderBufferDesc bokehPointsBufDesc;
		bokehPointsBufDesc.elementSize = 24;
		bokehPointsBufDesc.numElements = inputTex->Desc().width * inputTex->Desc().height;
		bokehPointsBufDesc.cpuAccess = 0;
		bokehPointsBufDesc.structedByteStride = bokehPointsBufDesc.elementSize;
		bokehPointsBufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_UNORDERED_ACCESS | BUFFER_BIND_STRUCTURED;
		auto bokehBuffer = re->GetRenderFactory()->CreateBuffer(bokehPointsBufDesc, nullptr);
		_fx->VariableByName("bokehPointsBuffer")->AsUAV()->SetValue(
			bokehBuffer->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, bokehPointsBufDesc.numElements, BUFFER_UAV_APPEND));

		_fx->VariableByName("computeBokehInTex")->AsShaderResource()->SetValue(inputTex->CreateTextureView());

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(resultTex->Desc().width);
		vp.height = static_cast<float>(resultTex->Desc().height);
		rc->SetViewport(vp);

		rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
		rc->SetRenderInput(CommonInput::QuadInput());

		_fx->TechniqueByName("ComputeBokehPoints")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("ComputeBokehPoints")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		return std::make_pair(bokehBuffer, resultTex);
	}

	Ptr<Texture> BokehDepthOfField::RenderBokeh(const std::initializer_list<Ptr<Texture>> & targets, const Ptr<RenderBuffer> & bokehPointsBuffer)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		//Render Bokehs

		auto texDesc = (*targets.begin())->Desc();
		auto resultTex = re->GetRenderFactory()->GetTexturePooled(texDesc);
		rc->ClearRenderTargets({ resultTex->CreateTextureView() }, 0.0f);

		RenderBufferDesc indirectArgsBufDesc = bokehPointsBuffer->Desc();
		indirectArgsBufDesc.elementSize = 16;
		indirectArgsBufDesc.numElements = 1;
		indirectArgsBufDesc.bindFlag = BUFFER_BIND_INDIRECT_ARGS;
		indirectArgsBufDesc.structedByteStride = 0;

		uint32_t initData[] = { 0, 1, 0, 0 };

		auto indirectAgsBuffer = re->GetRenderFactory()->CreateBuffer(indirectArgsBufDesc, &initData[0]);

		bokehPointsBuffer->CopyStructureCountTo(indirectAgsBuffer, 0, 0, bokehPointsBuffer->Desc().numElements, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_APPEND);

		//auto & texDesc = (*targets.begin())->Desc();

		float2 texelSize = 1.0f / float2(static_cast<float>(texDesc.width), static_cast<float>(texDesc.height));
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

		return resultTex;
	}

	void BokehDepthOfField::Recombine(
		const std::vector<Ptr<Texture>> & nearBlurTex,
		const std::vector<Ptr<Texture>> & farBlurTex,
		const Ptr<Texture> & bokehTex,
		const ResourceView & target
		)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		_fx->VariableByName("nearBlurTex")->AsShaderResource()->SetValue(nearBlurTex[0]->CreateTextureView());
		_fx->VariableByName("farBlurTex")->AsShaderResource()->SetValue(farBlurTex[0]->CreateTextureView());
		_fx->VariableByName("bokehLayerTex")->AsShaderResource()->SetValue(bokehTex->CreateTextureView());
		//_fx->VariableByName("farBlurTex2")->AsShaderResource()->SetValue(farBlurTex[1]->CreateTextureView());
		//_fx->VariableByName("minMaxCoCTex")->AsShaderResource()->SetValue(minMaxCoCTex->CreateTextureView());

		//float maxBlur = 15.0f;
		_fx->VariableByName("maxCoC")->AsScalar()->SetValue(&_maxCoC);

		rc->SetRenderTargets({ target }, 0);
		rc->SetRenderInput(CommonInput::QuadInput());

		_fx->TechniqueByName("Recombine")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("Recombine")->PassByIndex(0)->UnBind();
	}
}