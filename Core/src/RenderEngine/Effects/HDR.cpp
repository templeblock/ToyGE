#include "ToyGE\RenderEngine\Effects\HDR.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"

namespace ToyGE
{
	HDR::HDR()
		: _brightPassThreshold(0.5f),
		_brightPassScaleParam(2.0f)
	{
		_hdrFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"HDR.xml");

		TextureDesc texDesc;
		texDesc.width = texDesc.height = texDesc.depth = 1;
		texDesc.mipLevels = texDesc.arraySize = 1;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_2D;

		_prevAvgAdaptedIlumTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		_avgAdaptedIlumTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTargets({ _avgAdaptedIlumTex->CreateTextureView() }, 0.0f);
	}

	void HDR::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);
		//auto sceneTex = re->GetRenderFactory()->GetTexturePooled(targetTex->Desc());
		//targetTex->CopyTo(sceneTex, 0, 0, 0, 0, 0, 0, 0);

		rc->SetDepthStencil(ResourceView());
		rc->SetRenderInput(CommonInput::QuadInput());

		auto sceneDownSampleTex = SceneDownSample(sharedEnviroment->GetView()->GetRenderResult());

		CalcIlluminace(sceneDownSampleTex);

		auto brightPass = BrightPass(sceneDownSampleTex);


		int32_t numBlurTex = 6;
		//Bloom DownSample
		std::vector<Ptr<Texture>> bloomDownSampleTexList;
		bloomDownSampleTexList.push_back(brightPass);
		for (int32_t i = 1; i < numBlurTex; ++i)
		{
			bloomDownSampleTexList.push_back(BloomDownSample(bloomDownSampleTexList.back()));
		}

		//Bloom Blur
		std::vector<Ptr<Texture>> blurTexList;
		for (int i = 0; i < numBlurTex; ++i)
		{
			auto texDesc = bloomDownSampleTexList[i]->Desc();
			auto blurTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
			Blur::GaussBlur(bloomDownSampleTexList[i], 0, 0, blurTex, 0, 0, 2, 1.0f);
			blurTexList.push_back(blurTex);
		}

		//Bloom UpSample
		for (int i = numBlurTex - 2; i >= 0; --i)
		{
			BloomUpSample(blurTexList[i + 1], blurTexList[i]);
		}

		//Streak
		auto texDesc = brightPass->Desc();
		texDesc.mipLevels = 1;
		auto streakTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto streakTexTmp = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		brightPass->CopyTo(streakTex, 0, 0, 0, 0, 0, 0, 0);
		for (int i = 0; i < 3; ++i)
		{
			Blur::GaussBlurX(streakTex, 0, 0, streakTexTmp, 0, 0, 10, 1.0f);
			using std::swap;
			swap(streakTex, streakTexTmp);
		}
		streakTexTmp->Release();

		Combine(sharedEnviroment->GetView()->GetRenderResult(), blurTexList[0], streakTex, sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView());

		sharedEnviroment->GetView()->FlipRenderTarget();

		//sceneTex->Release();
		sceneDownSampleTex->Release();
		brightPass->Release();
		for (auto & i : bloomDownSampleTexList)
			i->Release();
		for (auto & i : blurTexList)
			i->Release();
		streakTex->Release();
	}

	Ptr<Texture> HDR::SceneDownSample(const Ptr<Texture> & sceneTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = sceneTex->Desc();
		texDesc.mipLevels = 1;
		texDesc.width = texDesc.width / 4;
		texDesc.height = texDesc.height / 4;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		auto downSampleTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		_hdrFX->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());

		rc->SetRenderTargets({ downSampleTex->CreateTextureView() }, 0);

		/*RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);

		_hdrFX->TechniqueByName("SceneDownSample")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_hdrFX->TechniqueByName("SceneDownSample")->PassByIndex(0)->UnBind();*/

		RenderQuad(_hdrFX->TechniqueByName("SceneDownSample"));

		return downSampleTex;
	}

	void HDR::CalcIlluminace(const Ptr<Texture> & downSampleTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = downSampleTex->Desc();
		texDesc.format = RENDER_FORMAT_R32_FLOAT;
		texDesc.width = texDesc.height = 64;
		auto tex0 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto tex1 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		/*RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;*/

		//Initial
		/*vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);*/

		_hdrFX->VariableByName("sceneDownSampleTex")->AsShaderResource()->SetValue(downSampleTex->CreateTextureView());
		rc->SetRenderTargets({ tex0->CreateTextureView() }, 0);

		RenderQuad(_hdrFX->TechniqueByName("CalcIlluminace_Initial"));

		/*_hdrFX->TechniqueByName("CalcIlluminace_Initial")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_hdrFX->TechniqueByName("CalcIlluminace_Initial")->PassByIndex(0)->UnBind();*/

		//Reduce calculate ilumance
		float w = ceil(static_cast<float>(texDesc.width) / 4.0f);
		float h = ceil(static_cast<float>(texDesc.height) / 4.0f);
		while (w >= 1.0f || h >= 1.0f)
		{
			//vp.width = w;
			//vp.height = h;
			//rc->SetViewport(vp);

			_hdrFX->VariableByName("reduceTex")->AsShaderResource()->SetValue(tex0->CreateTextureView());

			rc->SetRenderTargets({ tex1->CreateTextureView() }, 0);

			RenderQuad(_hdrFX->TechniqueByName("CalcIlluminace_ReduceAccum"),
				0, 0, static_cast<int32_t>(w), static_cast<int32_t>(h));

			/*_hdrFX->TechniqueByName("CalcIlluminace_ReduceAccum")->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			_hdrFX->TechniqueByName("CalcIlluminace_ReduceAccum")->PassByIndex(0)->UnBind();*/

			tex0.swap(tex1);

			if (w == 1.0f && h == 1.0f)
				break;

			w = ceil(w / 4.0f);
			h = ceil(h / 4.0f);
		}

		//texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		//auto avgAdaptedIlumTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		std::swap(_prevAvgAdaptedIlumTex, _avgAdaptedIlumTex);

		auto elapsedTime = Global::GetInfo()->GetElapsedTime();
		_hdrFX->VariableByName("elapsedTime")->AsScalar()->SetValue(&elapsedTime);

		_hdrFX->VariableByName("ilumReduceTex")->AsShaderResource()->SetValue(tex0->CreateTextureView());
		_hdrFX->VariableByName("prevAvgAdaptedIlumTex")->AsShaderResource()->SetValue(_prevAvgAdaptedIlumTex->CreateTextureView());

		rc->SetRenderTargets({ _avgAdaptedIlumTex->CreateTextureView() }, 0);

		RenderQuad(_hdrFX->TechniqueByName("ComputeAvgAndAdaptedIlum"), 0, 0, 1, 1);

		tex0->Release();
		tex1->Release();

		/*texDesc.width = texDesc.height = 1;
		texDesc.bindFlag = 0;
		texDesc.cpuAccess = CPU_ACCESS_READ;
		auto resultDumpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto cpyBox = std::make_shared<Box>();
		cpyBox->left = cpyBox->top = cpyBox->front = 0;
		cpyBox->right = cpyBox->bottom = cpyBox->back = 1;
		tex0->CopyTo(resultDumpTex, 0, 0, 0, 0, 0, 0, 0, cpyBox);

		auto mappedRs = resultDumpTex->Map(MAP_READ, 0, 0);
		float result = *reinterpret_cast<float*>(mappedRs.pData);
		resultDumpTex->UnMap();
		
		_avgIlluminace = exp(result);
		float curIlum = _avgIlluminace;
		float adaptedIlum = _adaptedIlluminace;
		auto timeElapsed = DebugInfo::get_const_instance().GetValue<float>("TimeElapsed");
		_adaptedIlluminace = adaptedIlum + (curIlum - adaptedIlum) * (1.0f - pow(0.98f, 100 * timeElapsed * 0.001f));

		tex0->Release();
		tex1->Release();
		resultDumpTex->Release();*/
	}

	Ptr<Texture> HDR::BrightPass(const Ptr<Texture> & downSampleTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = downSampleTex->Desc();
		texDesc.mipLevels = 0;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
		auto brightPassTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		/*RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);*/

		_hdrFX->VariableByName("brightPassParams")->AsScalar()->SetValue(&float2(_brightPassThreshold, _brightPassScaleParam), sizeof(float2));
		//_hdrFX->VariableByName("illuminace")->AsScalar()->SetValue(&_adaptedIlluminace, sizeof(_adaptedIlluminace));
		_hdrFX->VariableByName("sceneDownSampleTex")->AsShaderResource()->SetValue(downSampleTex->CreateTextureView());

		_hdrFX->VariableByName("avgAdaptedIlumTex")->AsShaderResource()->SetValue(_avgAdaptedIlumTex->CreateTextureView());

		rc->SetRenderTargets({ brightPassTex->CreateTextureView() }, 0);

		RenderQuad(_hdrFX->TechniqueByName("BrightPass"));

		/*_hdrFX->TechniqueByName("BrightPass")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_hdrFX->TechniqueByName("BrightPass")->PassByIndex(0)->UnBind();*/

		//rc->SetRenderTargets({ ResourceView() }, 0);

		//brightPassTex->GenerateMips();

		return brightPassTex;
	}

	Ptr<Texture> HDR::BloomDownSample(const Ptr<Texture> & inTex)
	{
		auto texDesc = inTex->Desc();
		texDesc.mipLevels = 1;
		texDesc.width = std::max<int32_t>(1, texDesc.width / 2);
		texDesc.height = std::max<int32_t>(1, texDesc.height / 2);
		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		/*auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);*/

		auto texelSize = 1.0f / float2(static_cast<float>(inTex->Desc().width), static_cast<float>(inTex->Desc().height));
		_hdrFX->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		_hdrFX->VariableByName("bloomDownSampleInTex")->AsShaderResource()->SetValue(inTex->CreateTextureView());

		rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);

		RenderQuad(_hdrFX->TechniqueByName("BloomDownSample"));

		/*_hdrFX->TechniqueByName("BloomDownSample")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_hdrFX->TechniqueByName("BloomDownSample")->PassByIndex(0)->UnBind();*/

		//rc->SetViewport(preVP);

		return resultTex;
	}

	void HDR::BloomUpSample(const Ptr<Texture> & lowResTex, const Ptr<Texture> & highResTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		/*auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(highResTex->Desc().width);
		vp.height = static_cast<float>(highResTex->Desc().height);
		rc->SetViewport(vp);*/

		_hdrFX->VariableByName("bloomUpSampleInTex")->AsShaderResource()->SetValue(lowResTex->CreateTextureView());

		rc->SetRenderTargets({ highResTex->CreateTextureView() }, 0);

		RenderQuad(_hdrFX->TechniqueByName("BloomUpSample"));

		/*_hdrFX->TechniqueByName("BloomUpSample")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_hdrFX->TechniqueByName("BloomUpSample")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);*/
	}

	void HDR::Combine(
		const Ptr<Texture> & scene,
		const Ptr<Texture> & blurTex,
		const Ptr<Texture> & streakTex,
		const ResourceView & target)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto targetTex = std::static_pointer_cast<Texture>(target.resource);
		auto targetSize = targetTex->GetMipSize(target.subDesc.textureDesc.firstMipLevel);

		/*RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = static_cast<float>(std::get<0>(targetSize));
		vp.height = static_cast<float>(std::get<1>(targetSize));
		rc->SetViewport(vp);*/

		_hdrFX->VariableByName("sceneTex")->AsShaderResource()->SetValue(scene->CreateTextureView());
		_hdrFX->VariableByName("blurTex")->AsShaderResource()->SetValue(blurTex->CreateTextureView());
		_hdrFX->VariableByName("streakTex")->AsShaderResource()->SetValue(streakTex->CreateTextureView());

		rc->SetRenderTargets({ target }, 0);

		RenderQuad(_hdrFX->TechniqueByName("Combine"));

		/*_hdrFX->TechniqueByName("Combine")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_hdrFX->TechniqueByName("Combine")->PassByIndex(0)->UnBind();*/
	}
}