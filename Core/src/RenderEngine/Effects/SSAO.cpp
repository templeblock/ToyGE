#include "ToyGE\RenderEngine\Effects\SSAO.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Blur.h"

namespace ToyGE
{
	SSAO::SSAO()
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"SSAO.xml");
	}

	void SSAO::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto linearDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer1 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(1))->As<SharedParam<Ptr<Texture>>>()->GetValue();

		sharedEnviroment->GetView()->BindParams(_fx);

		/*auto camera = std::static_pointer_cast<PerspectiveCamera>(sharedEnviroment->GetView()->GetCamera());
		_fx->VariableByName("view")->AsScalar()->SetValue(&camera->ViewMatrix());
		_fx->VariableByName("proj")->AsScalar()->SetValue(&camera->ProjMatrix());
		float2 cameraNearFar = float2(camera->Near(), camera->Far());
		_fx->VariableByName("cameraNearFar")->AsScalar()->SetValue(&cameraNearFar);*/

		float invViewRatio = 1.0f;// / camera->AspectRatio();
		_fx->VariableByName("invViewRatio")->AsScalar()->SetValue(&invViewRatio);

		float aoRadius = 0.08f;
		_fx->VariableByName("aoRadius")->AsScalar()->SetValue(&aoRadius);

		int32_t numMips = 2;

		std::vector<Ptr<Texture>> normalTexs(numMips);
		normalTexs[0] = gbuffer1;
		std::vector<Ptr<Texture>> depthTexs(numMips);
		depthTexs[0] = linearDepthTex;

		for (int32_t i = 1; i < numMips; ++i)
		{
			DownSampleNormalDepth(normalTexs[i - 1], depthTexs[i - 1], normalTexs[i], depthTexs[i]);
		}

		std::vector<Ptr<Texture>> aoTexs(numMips);
		float radius = aoRadius * static_cast<float>(1 << (numMips - 1));
		_fx->VariableByName("aoRadius")->AsScalar()->SetValue(&radius);
		aoTexs.back() = RenderAOTex(normalTexs.back(), depthTexs.back(), nullptr, nullptr, nullptr, false, numMips == 1);

		for (int32_t i = numMips - 2; i >= 0; --i)
		{
			aoTexs[i] = RenderAOTex(normalTexs[i], depthTexs[i], aoTexs[i + 1], normalTexs[i + 1], depthTexs[i + 1], true, i == 0);
			float radius = aoRadius * static_cast<float>(1 << i);
			_fx->VariableByName("aoRadius")->AsScalar()->SetValue(&radius);
		}

		/*float depthDiffScale = (cameraNearFar.y - cameraNearFar.x) * 0.01f;

		auto texDesc = aoTexMip1->Desc();
		texDesc.width *= 2;
		texDesc.height *= 2;
		auto aoTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		BilateralUpSampling(
			aoTexMip1->CreateTextureView(),
			depthTexMip1->CreateTextureView(),
			linearDepthTex->CreateTextureView(),
			aoTex->CreateTextureView(), depthDiffScale);*/

		CrossBilateralBlur(aoTexs[0], linearDepthTex);

		CombineAO(aoTexs[0], sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView());
		//Transform(aoTexs[0]->CreateTextureView(), sharedEnviroment->GetRenderTarget(), COLOR_WRITE_R);

		/*normalTexMip1->Release();
		depthTexMip1->Release();
		aoTexMip1->Release();
		aoTex->Release();*/
		for (auto & i : normalTexs)
			i->Release();
		for (auto & i : depthTexs)
			i->Release();
		for (auto & i : aoTexs)
			i->Release();
	}

	void SSAO::DownSampleNormalDepth(
		const Ptr<Texture> & inNormalTex,
		const Ptr<Texture> & inDepthTex,
		Ptr<Texture> & outNormalTex,
		Ptr<Texture> & outDepthTex)
	{
		auto texDesc = inNormalTex->Desc();
		texDesc.width /= 2;
		texDesc.height /= 2;

		outNormalTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		texDesc.format = inDepthTex->Desc().format;
		outDepthTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		float w = static_cast<float>(texDesc.width);
		float h = static_cast<float>(texDesc.height);
		float4 texSize = float4(w, h, 1.0f / w, 1.0f / h);
		_fx->VariableByName("texSize")->AsScalar()->SetValue(&texSize);

		_fx->VariableByName("normalTex")->AsShaderResource()->SetValue(inNormalTex->CreateTextureView());
		_fx->VariableByName("depthTex")->AsShaderResource()->SetValue(inDepthTex->CreateTextureView());

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ outNormalTex->CreateTextureView(), outDepthTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("DownSampleNormalDepth"), 0, 0, texDesc.width, texDesc.height);
	}

	/*Ptr<Texture> SSAO::BilateralUpSampling(const Ptr<Texture> & aoTex, const Ptr<Texture> & lowResDepthTex, const Ptr<Texture> & highResDepthTex)
	{
		auto texDesc = aoTex->Desc();
		texDesc.width *= 2;
		texDesc.height *= 2;

		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		_fx->VariableByName("upSamplingInTex")->AsShaderResource()->SetValue(aoTex->CreateTextureView());
		_fx->VariableByName("lowResDepthTex")->AsShaderResource()->SetValue(lowResDepthTex->CreateTextureView());
		_fx->VariableByName("highResDepthTex")->AsShaderResource()->SetValue(highResDepthTex->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ resultTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("BilateralUpSampling"), 0, 0, texDesc.width, texDesc.height);

		return resultTex;
	}*/

	Ptr<Texture> SSAO::RenderAOTex(
		const Ptr<Texture> & normalTex,
		const Ptr<Texture> & depthTex,
		const Ptr<Texture> & preMipAOTex,
		const Ptr<Texture> & preMipNormalTex,
		const Ptr<Texture> & preMipDepthTex,
		bool bUpSampling,
		bool bFullRes)
	{
		float w = static_cast<float>(depthTex->Desc().width);
		float h = static_cast<float>(depthTex->Desc().height);
		float4 texSize = float4(w, h, 1.0f / w, 1.0f / h);
		_fx->VariableByName("texSize")->AsScalar()->SetValue(&texSize);

		_fx->VariableByName("depthTex")->AsShaderResource()->SetValue(depthTex->CreateTextureView());
		_fx->VariableByName("normalTex")->AsShaderResource()->SetValue(normalTex->CreateTextureView());

		_fx->VariableByName("aoTex")->AsShaderResource()->SetValue(preMipAOTex ? preMipAOTex->CreateTextureView() : ResourceView());
		_fx->VariableByName("preMipNormalTex")->AsShaderResource()->SetValue(preMipNormalTex ? preMipNormalTex->CreateTextureView() : ResourceView());
		_fx->VariableByName("preMipDepthTex")->AsShaderResource()->SetValue(preMipDepthTex ? preMipDepthTex->CreateTextureView() : ResourceView());

		auto texDesc = depthTex->Desc();
		texDesc.format = RENDER_FORMAT_R8_UNORM;
		auto aoTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ aoTex->CreateTextureView() }, 0);

		std::vector<MacroDesc> macros;
		if (bUpSampling)
			macros.push_back({ "UPSAMPLING", "" });
		if (bFullRes)
			macros.push_back({ "FULL_RES", "" });

		_fx->SetExtraMacros(macros);
		/*String techName = "RenderAO";
		if (bUpSampling)
		{
			if (bFullRes)
				techName = "RenderAOFullResUpSampling";
			else
				techName = "RenderAOUpSampling";
		}
		else if (bFullRes)
			techName = "RenderAOFullRes";*/

		RenderQuad(_fx->TechniqueByName("RenderAO"), 0, 0, texDesc.width, texDesc.height);

		return aoTex;
	}

	void SSAO::CrossBilateralBlur(const Ptr<Texture> & aoTex, const Ptr<Texture> & depthTex)
	{
		auto texDesc = aoTex->Desc();
		auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		float w = static_cast<float>(depthTex->Desc().width);
		float h = static_cast<float>(depthTex->Desc().height);
		float4 texSize = float4(w, h, 1.0f / w, 1.0f / h);
		_fx->VariableByName("texSize")->AsScalar()->SetValue(&texSize);

		auto & gaussWeightsTable = Blur::GaussTable(3);
		//_fx->VariableByName("gaussWeightsTable")->AsScalar()->SetValue(&gaussWeightsTable[0], sizeof(gaussWeightsTable[0]) * gaussWeightsTable.size());

		_fx->VariableByName("depthTex")->AsShaderResource()->SetValue(depthTex->CreateTextureView());

		_fx->VariableByName("blurInTex")->AsShaderResource()->SetValue(aoTex->CreateTextureView());
		rc->SetRenderTargets({ tmpTex->CreateTextureView() }, 0);
		RenderQuad(_fx->TechniqueByName("CrossBilateralBlurX"), 0, 0, texDesc.width, texDesc.height);

		_fx->VariableByName("blurInTex")->AsShaderResource()->SetValue(tmpTex->CreateTextureView());
		rc->SetRenderTargets({ aoTex->CreateTextureView() }, 0);
		RenderQuad(_fx->TechniqueByName("CrossBilateralBlurY"), 0, 0, texDesc.width, texDesc.height);

		tmpTex->Release();
	}

	void SSAO::CombineAO(const Ptr<Texture> & aoTex, const ResourceView & target)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		_fx->VariableByName("aoTex")->AsShaderResource()->SetValue(aoTex->CreateTextureView());

		rc->SetRenderTargets({ target }, 0);

		RenderQuad(_fx->TechniqueByName("CombineAO"), 0, 0, aoTex->Desc().width, aoTex->Desc().height);
	}
}
