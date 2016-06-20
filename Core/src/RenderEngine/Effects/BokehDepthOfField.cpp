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

		auto cocTex = view->GetViewRenderContext()->GetSharedTexture("CoC");

		auto tileMaxTexRef = TileMax(cocTex, sceneLinearClipDepth);
		auto tileMaxTex = tileMaxTexRef->Get()->Cast<Texture>();

		PooledTextureRef dividingTexRef, downSampleColorTexRef, downSampleDepthTexRef;

		PreDividing(tileMaxTex, cocTex, sceneTex, sceneLinearClipDepth, dividingTexRef, downSampleColorTexRef, downSampleDepthTexRef);
		auto dividingTex = dividingTexRef->Get()->Cast<Texture>();
		auto downSampleColorTex = downSampleColorTexRef->Get()->Cast<Texture>();
		auto downSampleDepthTex = downSampleDepthTexRef->Get()->Cast<Texture>();

		PooledTextureRef halfResBlurTexRef, halfResAlphaTexRef;
		DOFBlur(downSampleColorTex, cocTex, tileMaxTex, dividingTex, downSampleDepthTex, halfResBlurTexRef, halfResAlphaTexRef);
		auto halfResBlurTex = halfResBlurTexRef->Get()->Cast<Texture>();
		auto halfResAlphaTex = halfResAlphaTexRef->Get()->Cast<Texture>();

		auto dofResultTexRef = UpSampling(sceneTex, cocTex, tileMaxTex, halfResBlurTex, halfResAlphaTex);
		auto dofReultTex = dofResultTexRef->Get()->Cast<Texture>();

		// Temporal AA for SSR
		auto dofAAedResultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, dofReultTex->GetDesc() });
		auto dofAAedResultTex = dofAAedResultTexRef->Get()->Cast<Texture>();
		{
			auto adaptedExposureScale = view->GetViewRenderContext()->GetSharedTexture("AdaptedExposureScale");

			float2 offsets[5] =
			{
				float2(0.0f, 0.0f),
				float2(-1.0f, 0.0f),
				float2(1.0f, 0.0f),
				float2(0.0f, -1.0f),
				float2(0.0f, 1.0f),
			};
			float filterWeights[5];
			float weightsSum = 0.0f;
			for (int i = 0; i < 5; ++i)
			{
				float2 offset = offsets[i] - float2(0.5f, -0.5f) * view->temporalAAJitter;

				//filterWeights[i] = CatmullRom(offset.x()) * CatmullRom(offset.y());
				offset.x() *= 1.0f + 0.0f * 0.5f;
				offset.y() *= 1.0f + 0.0f * 0.5f;
				filterWeights[i] = exp(-2.29f * (offset.x() * offset.x() + offset.y() * offset.y()));

				weightsSum += filterWeights[i];
			}
			for (auto & i : filterWeights)
				i /= weightsSum;

			std::map<String, String> macros;
			macros["TAA_DYNAMIC"] = "0";
			macros["TAA_HISTORY_BICUBIC"] = "0";

			auto ps = Shader::FindOrCreate<TemporalAAPS>(macros);

			view->BindShaderParams(ps);

			ps->SetScalar("texSize", dofAAedResultTex->GetTexSize());
			ps->SetScalar("neighborFilterWeights", filterWeights, (int)sizeof(float) * 5);
			ps->SetScalar("frameCount", (uint32_t)Global::GetInfo()->frameCount);
			ps->SetScalar("lerpFactor", 0.125f);
			//ps->SetSRV("linearDepth", sceneLinearClipDepth->GetShaderResourceView());
			//ps->SetSRV("sceneDepth", sceneClipDepth->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
			ps->SetSRV("sceneTex", dofReultTex->GetShaderResourceView());
			//ps->SetSRV("velocityTex", velocityTex->GetShaderResourceView());
			//ps->SetSRV("adaptedExposureScale", adaptedExposureScale->GetShaderResourceView());
			if (_preDofResultRef)
				ps->SetSRV("historyTex", _preDofResultRef->Get()->Cast<Texture>()->GetShaderResourceView());
			else
				ps->SetSRV("historyTex", nullptr);
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

			ps->Flush();

			DrawQuad(
			{ dofAAedResultTex->GetRenderTargetView(0, 0, 1) });

			_preDofResultRef = dofAAedResultTexRef;
		}

		view->GetViewRenderContext()->SetSharedResource("RenderResult", dofAAedResultTexRef);
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

	PooledTextureRef BokehDepthOfField::DownSampleColor(const Ptr<Texture> & tex)
	{
		auto texDesc = tex->GetDesc();
		texDesc.width /= 2;
		texDesc.height /= 2;

		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto resultTex = resultTexRef->Get()->Cast<Texture>();

		Transform(tex->GetShaderResourceView(), resultTex->GetRenderTargetView(0, 0, 1));

		return resultTexRef;
	}

	PooledTextureRef BokehDepthOfField::DownSampleDepth(const Ptr<Texture> & tex)
	{
		auto texDesc = tex->GetDesc();
		texDesc.width /= 2;
		texDesc.height /= 2;

		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto resultTex = resultTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<DownSampleDepthPS>();
		ps->SetScalar("downSampleOffset", float2(0.25f / (float)texDesc.width, 0.25f / (float)texDesc.height));
		ps->SetSRV("linearDepthTex", tex->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->Flush();

		DrawQuad({ resultTex->GetRenderTargetView(0, 0, 1) });

		return resultTexRef;
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

	void BokehDepthOfField::PreDividing(
		const Ptr<Texture> & tileMaxTex,
		const Ptr<Texture> & cocTex,
		const Ptr<Texture> & sceneColorTex,
		const Ptr<Texture> & sceneDepthTex,
		PooledTextureRef & outDividingTex,
		PooledTextureRef & outDownSampleColorTex,
		PooledTextureRef & outDownSampleDepthTex)
	{
		auto texDesc = sceneColorTex->GetDesc();
		texDesc.width /= 2;
		texDesc.height /= 2;

		outDownSampleColorTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto downSampleColorTex = outDownSampleColorTex->Get()->Cast<Texture>();

		texDesc.format = sceneDepthTex->GetDesc().format;
		outDownSampleDepthTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto downSampleDepthTex = outDownSampleDepthTex->Get()->Cast<Texture>();

		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		outDividingTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto dividingTex = outDividingTex->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<PreDividingPS>();
		ps->SetScalar("downSampleOffset", float2(0.25f / (float)texDesc.width, 0.25f / (float)texDesc.height));
		ps->SetScalar("texSize", sceneColorTex->GetTexSize());
		ps->SetScalar("frameCount", Global::GetInfo()->frameCount);

		ps->SetSRV("tileMaxTex", tileMaxTex->GetShaderResourceView());
		ps->SetSRV("cocTex", cocTex->GetShaderResourceView());
		ps->SetSRV("sceneTex", sceneColorTex->GetShaderResourceView());
		ps->SetSRV("linearDepthTex", sceneDepthTex->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());
		ps->Flush();

		DrawQuad({ 
			dividingTex->GetRenderTargetView(0, 0, 1), 
			downSampleColorTex->GetRenderTargetView(0, 0, 1), 
			downSampleDepthTex->GetRenderTargetView(0, 0, 1) });
	}

	void BokehDepthOfField::DOFBlur(
		const Ptr<Texture> & inTex,
		const Ptr<Texture> & cocTex,
		const Ptr<Texture> & tileMaxTex,
		const Ptr<Texture> & dividingTex,
		const Ptr<Texture> & sceneDepthTex,
		PooledTextureRef & outHalfResBlurTex,
		PooledTextureRef & outHalfResAlphaTex)
	{
		auto texDesc = inTex->GetDesc();

		/*auto blurTempTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto blurTempTex = blurTempTexRef->Get()->Cast<Texture>();*/
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		outHalfResBlurTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto blurTex = outHalfResBlurTex->Get()->Cast<Texture>();

		texDesc.format = RENDER_FORMAT_R8_UNORM;
		outHalfResAlphaTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto alphaTex = outHalfResAlphaTex->Get()->Cast<Texture>();

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

			DrawQuad({ blurTex->GetRenderTargetView(0, 0, 1), alphaTex->GetRenderTargetView(0, 0, 1) });
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

		//return blurTexRef;
	}

	PooledTextureRef BokehDepthOfField::UpSampling(
		const Ptr<Texture> & sceneColorTex,
		const Ptr<Texture> & cocTex,
		const Ptr<Texture> & tileMaxTex,
		const Ptr<Texture> & halfResBlurTex,
		const Ptr<Texture> & halfResAlphaTex)
	{
		auto texDesc = sceneColorTex->GetDesc();

		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto resultTex = resultTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<DOFUpSamplingPS>();
		ps->SetScalar("texSize", sceneColorTex->GetTexSize());
		ps->SetScalar("frameCount", Global::GetInfo()->frameCount);

		ps->SetSRV("sceneTex", sceneColorTex->GetShaderResourceView());
		ps->SetSRV("cocTex", cocTex->GetShaderResourceView());
		ps->SetSRV("tileMaxTex", tileMaxTex->GetShaderResourceView());
		ps->SetSRV("halfResColorTex", halfResBlurTex->GetShaderResourceView());
		ps->SetSRV("halfResAlphaTex", halfResAlphaTex->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ resultTex->GetRenderTargetView(0, 0, 1) });

		return resultTexRef;
	}
}