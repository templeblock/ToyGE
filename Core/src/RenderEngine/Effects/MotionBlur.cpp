#include "ToyGE\RenderEngine\Effects\MotionBlur.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\GlobalInfo.h"

namespace ToyGE
{
	MotionBlur::MotionBlur()
		: _exposureTime(5.0f),
		_maxVelocityLength(16),
		_mbNumSamples(16)
	{
	}

	void MotionBlur::Render(const Ptr<RenderView> & view)
	{
		auto rawVelocity = view->GetViewRenderContext()->GetSharedTexture("Velocity");
		if (!rawVelocity)
			return;

		auto velocityRef = InitVelocityMap(rawVelocity);

		auto tileMaxTexRef = TileMax(velocityRef->Get()->Cast<Texture>());

		auto neighborMaxTexRef = NeighborMax(tileMaxTexRef->Get()->Cast<Texture>());

		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
		auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");

		auto targetTexDesc = sceneTex->GetDesc();
		auto targetTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, targetTexDesc });

		Blur(
			sceneTex, 
			sceneLinearClipDepth, 
			velocityRef->Get()->Cast<Texture>(), 
			neighborMaxTexRef->Get()->Cast<Texture>(), 
			targetTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1));

		view->GetViewRenderContext()->SetSharedResource("RenderResult", targetTexRef);
	}

	PooledTextureRef MotionBlur::InitVelocityMap(const Ptr<Texture> & rawVelocityTex)
	{
		auto texDesc = rawVelocityTex->GetDesc();
		auto velocityMap = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		auto ps = Shader::FindOrCreate<InitVelocityMapPS>();

		float velocityScale = _exposureTime * Global::GetInfo()->fps;
		ps->SetScalar("velocityScale", velocityScale);

		ps->SetSRV("velocityTex", rawVelocityTex->GetShaderResourceView());

		ps->Flush();

		DrawQuad({ velocityMap->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });

		return velocityMap;
	}

	PooledTextureRef MotionBlur::TileMax(const Ptr<Texture> & velocityTex)
	{
		auto texDesc = velocityTex->GetDesc();
		texDesc.width = (texDesc.width + _maxVelocityLength - 1) / _maxVelocityLength;
		auto tileMaxTexTmpRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		texDesc.height = (texDesc.height + _maxVelocityLength - 1) / _maxVelocityLength;
		auto tileMaxTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		{
			auto ps = Shader::FindOrCreate<TileMaxXPS>();

			ps->SetScalar("texSize", velocityTex->GetTexSize());
			ps->SetSRV("velocityTex", velocityTex->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			ps->Flush();

			DrawQuad({ tileMaxTexTmpRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });
		}

		{
			auto ps = Shader::FindOrCreate<TileMaxYPS>();

			ps->SetScalar("texSize", tileMaxTexTmpRef->Get()->Cast<Texture>()->GetTexSize());
			ps->SetSRV("velocityTex", tileMaxTexTmpRef->Get()->Cast<Texture>()->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			ps->Flush();

			DrawQuad({ tileMaxTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });
		}

		return tileMaxTexRef;
	}

	PooledTextureRef MotionBlur::NeighborMax(const Ptr<Texture> & tileMaxTex)
	{
		auto texDesc = tileMaxTex->GetDesc();
		auto neighborMaxTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		auto ps = Shader::FindOrCreate<NeighborMaxPS>();

		ps->SetScalar("texSize", tileMaxTex->GetTexSize());
		ps->SetSRV("tileMaxTex", tileMaxTex->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ neighborMaxTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });

		return neighborMaxTexRef;
	}

	void MotionBlur::Blur(
		const Ptr<Texture> & sceneTex,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & velocityTex,
		const Ptr<Texture> & neighborMaxTex,
		const Ptr<RenderTargetView> & target)
	{
		auto texDesc = sceneTex->GetDesc();
		texDesc.mipLevels = 1;
		auto tmpTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		auto ps = Shader::FindOrCreate<MotionBlurPS>();

		{
			ps->SetScalar("texSize", sceneTex->GetTexSize());
			ps->SetScalar("frameCount", Global::GetInfo()->frameCount);
			ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
			ps->SetSRV("linearDepthTex", linearDepthTex->GetShaderResourceView());
			ps->SetSRV("velocityTex", velocityTex->GetShaderResourceView());
			ps->SetSRV("neighborMaxTex", neighborMaxTex->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			ps->Flush();

			DrawQuad({ tmpTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });
		}

		{
			ps->SetScalar("texSize", sceneTex->GetTexSize());
			ps->SetScalar("frameCount", Global::GetInfo()->frameCount);
			ps->SetSRV("sceneTex", tmpTex->Get()->Cast<Texture>()->GetShaderResourceView());
			ps->SetSRV("linearDepthTex", linearDepthTex->GetShaderResourceView());
			ps->SetSRV("velocityTex", velocityTex->GetShaderResourceView());
			ps->SetSRV("neighborMaxTex", neighborMaxTex->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			ps->Flush();

			DrawQuad({ target });
		}
	}
}