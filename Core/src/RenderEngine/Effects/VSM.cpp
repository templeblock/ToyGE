#include "ToyGE\RenderEngine\Effects\VSM.h"
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
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\Effects\CascadedShadowDepthTechnique.h"
#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	ShadowRenderTechniqueVSM::ShadowRenderTechniqueVSM()
		: _softness(0.06f),
		_vsmBias(0.1f),
		_reduceBlending(0.5f)
	{
	}

	Ptr<Texture> ShadowRenderTechniqueVSM::ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light)
	{
		auto texDesc = shadowMap->Desc();
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		auto convertTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto convertFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"VSMConvert.xml");
		ConvertShadowMap(shadowMap, convertTex, convertFX, "VSMConvert");

		shadowMap->Release();

		return ShadowRenderTechniqueVSM::FilterShadowMap(convertTex, light);
	}

	Ptr<Texture> ShadowRenderTechniqueVSM::FilterShadowMap(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light)
	{
		auto depthTech = light->GetShadowTechnique()->DepthTechnique();

		int32_t blurRadius = 4;

		int32_t shadowMapSize = light->GetShadowTechnique()->ShadowTexSize().x;
		if (light->Type() == LIGHT_DIRECTIONAL && depthTech->IsCSM())
		{
			auto pssm = std::static_pointer_cast<PSSMDepthTechnique>(light->GetShadowTechnique()->DepthTechnique());
			float blurBaseFac = pssm->SplitMinMax(0).second.x - pssm->SplitMinMax(0).first.x;

			auto firstSplitSize = (pssm->SplitMinMax(0).second - pssm->SplitMinMax(0).first).v(VEC_X, VEC_Y);
			float baseSize = std::min<float>(firstSplitSize.x, firstSplitSize.y);

			for (int32_t arrayIndex = 0; arrayIndex < shadowMap->Desc().arraySize; ++arrayIndex)
			{
				auto size = (pssm->SplitMinMax(arrayIndex).second - pssm->SplitMinMax(arrayIndex).first).v(VEC_X, VEC_Y);

				auto softPixels = float2(static_cast<float>(shadowMapSize)) / size * _softness;
				float2 sampleOffsetScale = softPixels / static_cast<float>(blurRadius * 2 + 1);

				Blur::BoxBlur(shadowMap, 0, arrayIndex, shadowMap, 0, arrayIndex, blurRadius, sampleOffsetScale);
			}
		}
		else
		{
			int32_t num2DArrays = shadowMap->Desc().type == TEXTURE_CUBE ? shadowMap->Desc().arraySize * 6 : shadowMap->Desc().arraySize;

			for (int i = 0; i < num2DArrays; ++i)
				Blur::GaussBlur(shadowMap, 0, i, shadowMap, 0, i, blurRadius, 1.5f);
		}

		return shadowMap;
	}
	
	void ShadowRenderTechniqueVSM::BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		fx->AddExtraMacro("SHADOW_TYPE", "SHADOW_TYPE_VSM");
	}

	void ShadowRenderTechniqueVSM::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		ShadowRenderTechnique::BindParams(fx, light, shadowMap);

		fx->VariableByName("vsmBias")->AsScalar()->SetValue(&_vsmBias);
		fx->VariableByName("vsmReduceBlending")->AsScalar()->SetValue(&_reduceBlending);
	}

	void ShadowRenderTechniqueVSM::ConvertShadowMap(const Ptr<Texture> & inTex, const Ptr<Texture> & outTex, const Ptr<RenderEffect> & fx, const String & techniqueName)
	{
		//auto convertFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource("VSMConvert.xml");

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(inTex->Desc().width);
		vp.height = static_cast<float>(inTex->Desc().height);
		rc->SetViewport(vp);

		int32_t num2DArrays = inTex->Desc().type == TEXTURE_CUBE ? inTex->Desc().arraySize * 6 : inTex->Desc().arraySize;

		for (int32_t arrayIndex = 0; arrayIndex < num2DArrays; ++arrayIndex)
		{
			rc->SetDepthStencil(ResourceView());
			rc->SetRenderTargets({ outTex->CreateTextureView(0, 1, arrayIndex, 1) }, 0);
			rc->SetRenderInput(CommonInput::QuadInput());
			fx->VariableByName("inTex")->AsShaderResource()->SetValue(inTex->CreateTextureView(0, 1, arrayIndex, 1));

			fx->TechniqueByName(techniqueName)->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			fx->TechniqueByName(techniqueName)->PassByIndex(0)->UnBind();
		}

		rc->SetViewport(preVP);
	}


	/*
	EVSM2
	*/
	Ptr<Texture> ShadowRenderTechniqueEVSM2::ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light)
	{
		auto texDesc = shadowMap->Desc();
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		auto convertTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto convertFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"VSMConvert.xml");
		convertFX->SetExtraMacros({ { "EVSM_2", "" } });
		ConvertShadowMap(shadowMap, convertTex, convertFX, "EVSMConvert");

		shadowMap->Release();

		return ShadowRenderTechniqueVSM::FilterShadowMap(convertTex, light);
	}

	void ShadowRenderTechniqueEVSM2::BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		fx->AddExtraMacro("SHADOW_TYPE", "SHADOW_TYPE_EVSM2");
	}


	/*
	EVSM4
	*/
	Ptr<Texture> ShadowRenderTechniqueEVSM4::ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light)
	{
		auto texDesc = shadowMap->Desc();
		texDesc.format = RENDER_FORMAT_R32G32B32A32_FLOAT;
		auto convertTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto convertFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"VSMConvert.xml");
		convertFX->SetExtraMacros({});
		ConvertShadowMap(shadowMap, convertTex, convertFX, "EVSMConvert");

		shadowMap->Release();

		return ShadowRenderTechniqueVSM::FilterShadowMap(convertTex, light);
	}

	void ShadowRenderTechniqueEVSM4::BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		fx->AddExtraMacro("SHADOW_TYPE", "SHADOW_TYPE_EVSM4");
	}

	/*
	SAVSM
	*/
	float ShadowRenderTechniqueSAVSM::_defaultFilterSize = 7.0f;

	ShadowRenderTechniqueSAVSM::ShadowRenderTechniqueSAVSM()
	{
		SetFilterSize(_defaultFilterSize);
	}

	Ptr<Texture> ShadowRenderTechniqueSAVSM::ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light)
	{
		if (light->Type() == LIGHT_DIRECTIONAL
			|| light->Type() == LIGHT_POINT)
			return nullptr;

		//_satTex = SAT(shadowMap);

		return SAT(shadowMap);
	}

	void ShadowRenderTechniqueSAVSM::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		if (light->Type() == LIGHT_DIRECTIONAL
			|| light->Type() == LIGHT_POINT)
			return;

		ShadowRenderTechnique::BindParams(fx, light, shadowMap);

		//BindShadowTex(shadowMap, fx);
		fx->VariableByName("savsmFilterSize")->AsScalar()->SetValue(&_filterSize, sizeof(_filterSize));
	}
}