#include "ToyGE\RenderEngine\Effects\ImageBasedLensFlare.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\TextureAsset.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"

namespace ToyGE
{
	ImageBasedLensFlare::ImageBasedLensFlare()
		: _brightPassThreshold(0.5f),
		_spriteThreshold(1.0f),
		_flareIntensity(6.0f)
	{
	}

	void ImageBasedLensFlare::Render(const Ptr<RenderView> & view)
	{
		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		auto brightPassTexRef = BrightPass(sceneTex);

		auto setupTexRef = Setup(brightPassTexRef->Get()->Cast<Texture>());

		LensBlur(setupTexRef->Get()->Cast<Texture>(), sceneTex);
	}

	PooledTextureRef ImageBasedLensFlare::BrightPass(const Ptr<Texture> & sceneTex)
	{
		auto brightPassTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneTex->GetDesc() });

		auto ps = Shader::FindOrCreate<LensBlurBrightPassPS>();

		ps->SetScalar("brightPassThreshold", _brightPassThreshold);
		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ brightPassTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });

		return brightPassTexRef;
	}

	PooledTextureRef ImageBasedLensFlare::Setup(const Ptr<Texture> & brightPassTex)
	{
		auto setupTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, brightPassTex->GetDesc() });
		auto setupTex = setupTexRef->Get()->Cast<Texture>();

		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTarget(setupTex->GetRenderTargetView(0, 0, 1), 0.0f);

		auto ps = Shader::FindOrCreate<LensFlareSetupPS>();

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(
			BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

		int2 texSize = int2(setupTex->GetDesc().width / 2, setupTex->GetDesc().height / 2);
		int2 center = texSize;

		for (int i = 0; i < 3; ++i)
		{
			int2 xy = center - texSize / 2;

			ps->SetSRV("brightPassTex", brightPassTex->GetShaderResourceView());
			ps->SetSampler("linearSampler", SamplerTemplate<>::Get());
			ps->Flush();

			DrawQuad({ setupTex->GetRenderTargetView(0, 0, 1) }, 
				(float)xy.x(), (float)xy.y(), (float)texSize.x(), (float)texSize.y());

			texSize /= 2;
		}

		for (int i = 0; i < 5; ++i)
		{
			int2 xy = center - texSize / 2;

			ps->SetSRV("brightPassTex", brightPassTex->GetShaderResourceView());
			ps->SetSampler("linearSampler", SamplerTemplate<>::Get());
			ps->Flush();

			DrawQuad({ setupTex->GetRenderTargetView(0, 0, 1) }, 
				(float)xy.x(), (float)xy.y(), (float)texSize.x(), (float)texSize.y(),
				1.0f, 1.0f, -1.0f, -1.0f);

			texSize *= 2;
		}

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);

		return setupTexRef;
	}

	void ImageBasedLensFlare::LensBlur(const Ptr<Texture> & setupTex, const Ptr<Texture> & target)
	{
		int32_t tileSize = 9;

		//Extract Sprite Points
		int32_t extractWidth = (setupTex->GetDesc().width + tileSize - 1) / tileSize;
		int32_t extractHeight = (setupTex->GetDesc().height + tileSize - 1) / tileSize;

		RenderBufferDesc spPointsBufDesc;
		spPointsBufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_UNORDERED_ACCESS;
		spPointsBufDesc.elementSize = sizeof(float2) + sizeof(float3);
		spPointsBufDesc.numElements = extractWidth * extractHeight;
		spPointsBufDesc.cpuAccess = 0;
		spPointsBufDesc.bStructured = true;

		auto spPointsBufRef = BufferPool::Instance().FindFree(spPointsBufDesc);
		auto spPointsBuf = spPointsBufRef->Get()->Cast<RenderBuffer>();

		{
			auto ps = Shader::FindOrCreate<ExtractSpritePointsPS>();

			ps->SetScalar("spriteThreshold", _spriteThreshold);
			ps->SetSRV("setupTex", setupTex->GetShaderResourceView());
			ps->SetUAV("spPointsBuf", spPointsBuf->GetUnorderedAccessView(0, 0, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_APPEND));
			ps->Flush();

			DrawQuad({}, 0.0f, 0.0f, (float)extractWidth, (float)extractHeight);
		}

		//Render Sprites
		if (!_indirectAgsBuf)
		{
			RenderBufferDesc indirectArgsBufDesc;
			indirectArgsBufDesc.bindFlag = BUFFER_BIND_INDIRECT_ARGS;
			indirectArgsBufDesc.elementSize = 16;
			indirectArgsBufDesc.numElements = 1;
			indirectArgsBufDesc.cpuAccess = 0;
			indirectArgsBufDesc.bStructured = false;

			uint32_t initData[] = { 0, 1, 0, 0 };

			_indirectAgsBuf = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
			_indirectAgsBuf->SetDesc(indirectArgsBufDesc);
			_indirectAgsBuf->Init(initData);
		}

		spPointsBuf->CopyStructureCountTo(_indirectAgsBuf, 0, 0, spPointsBuf->GetDesc().numElements, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_APPEND);

		{
			auto vs = Shader::FindOrCreate<LensBlurVS>();
			auto gs = Shader::FindOrCreate<LensBlurGS>();
			auto ps = Shader::FindOrCreate<LensBlurPS>();

			vs->SetScalar("texSize", target->GetTexSize());
			gs->SetScalar("texSize", target->GetTexSize());

			gs->SetScalar("flareIntensity", _flareIntensity);

			vs->SetSRV("spPointsRenderBuf", spPointsBuf->GetShaderResourceView(0, 0, RENDER_FORMAT_UNKNOWN));

			auto lensTexAsset = Asset::Find<TextureAsset>("Textures/Bokeh_Circle.dds");
			if (!lensTexAsset->IsInit())
				lensTexAsset->Init();
			auto lensTex = lensTexAsset->GetTexture();
			ps->SetSRV("lensTex", lensTex->GetShaderResourceView());

			ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

			vs->Flush();
			gs->Flush();
			ps->Flush();

			auto rc = Global::GetRenderEngine()->GetRenderContext();

			rc->SetVertexBuffer({});
			rc->SetIndexBuffer(nullptr);
			rc->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);

			rc->SetViewport(GetTextureQuadViewport(target));

			rc->SetRenderTargets({ target->GetRenderTargetView(0, 0, 1) });
			rc->SetDepthStencil(nullptr);
			rc->SetDepthStencilState(DepthStencilStateTemplate<false>::Get());

			rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_SRC_ALPHA, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

			rc->DrawInstancedIndirect(_indirectAgsBuf, 0);

			rc->ResetShader(SHADER_GS);
			rc->SetBlendState(nullptr);
		}
	}
}