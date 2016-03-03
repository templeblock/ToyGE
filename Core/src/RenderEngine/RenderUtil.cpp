#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderFormat.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"
#include "ToyGE\RenderEngine\RenderView.h"

namespace ToyGE
{
	uint32_t GetRenderFormatNumBits(RenderFormat fmt)
	{
		switch (fmt)
		{
		case RENDER_FORMAT_R32G32B32A32_TYPELESS:
		case RENDER_FORMAT_R32G32B32A32_FLOAT:
		case RENDER_FORMAT_R32G32B32A32_UINT:
		case RENDER_FORMAT_R32G32B32A32_SINT:
			return 128;

		case RENDER_FORMAT_R32G32B32_TYPELESS:
		case RENDER_FORMAT_R32G32B32_FLOAT:
		case RENDER_FORMAT_R32G32B32_UINT:
		case RENDER_FORMAT_R32G32B32_SINT:
			return 96;

		case RENDER_FORMAT_R16G16B16A16_TYPELESS:
		case RENDER_FORMAT_R16G16B16A16_FLOAT:
		case RENDER_FORMAT_R16G16B16A16_UNORM:
		case RENDER_FORMAT_R16G16B16A16_UINT:
		case RENDER_FORMAT_R16G16B16A16_SNORM:
		case RENDER_FORMAT_R16G16B16A16_SINT:
		case RENDER_FORMAT_R32G32_TYPELESS:
		case RENDER_FORMAT_R32G32_FLOAT:
		case RENDER_FORMAT_R32G32_UINT:
		case RENDER_FORMAT_R32G32_SINT:
		case RENDER_FORMAT_R32G8X24_TYPELESS:
		case RENDER_FORMAT_D32_FLOAT_S8X24_UINT:
		case RENDER_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case RENDER_FORMAT_X32_TYPELESS_G8X24_UINT:
			return 64;

		case RENDER_FORMAT_R10G10B10A2_TYPELESS:
		case RENDER_FORMAT_R10G10B10A2_UNORM:
		case RENDER_FORMAT_R10G10B10A2_UINT:
		case RENDER_FORMAT_R11G11B10_FLOAT:
		case RENDER_FORMAT_R8G8B8A8_TYPELESS:
		case RENDER_FORMAT_R8G8B8A8_UNORM:
		case RENDER_FORMAT_R8G8B8A8_UNORM_SRGB:
		case RENDER_FORMAT_R8G8B8A8_UINT:
		case RENDER_FORMAT_R8G8B8A8_SNORM:
		case RENDER_FORMAT_R8G8B8A8_SINT:
		case RENDER_FORMAT_R16G16_TYPELESS:
		case RENDER_FORMAT_R16G16_FLOAT:
		case RENDER_FORMAT_R16G16_UNORM:
		case RENDER_FORMAT_R16G16_UINT:
		case RENDER_FORMAT_R16G16_SNORM:
		case RENDER_FORMAT_R16G16_SINT:
		case RENDER_FORMAT_R32_TYPELESS:
		case RENDER_FORMAT_D32_FLOAT:
		case RENDER_FORMAT_R32_FLOAT:
		case RENDER_FORMAT_R32_UINT:
		case RENDER_FORMAT_R32_SINT:
		case RENDER_FORMAT_R24G8_TYPELESS:
		case RENDER_FORMAT_D24_UNORM_S8_UINT:
		case RENDER_FORMAT_R24_UNORM_X8_TYPELESS:
		case RENDER_FORMAT_X24_TYPELESS_G8_UINT:
		case RENDER_FORMAT_R9G9B9E5_SHAREDEXP:
		case RENDER_FORMAT_R8G8_B8G8_UNORM:
		case RENDER_FORMAT_G8R8_G8B8_UNORM:
		case RENDER_FORMAT_B8G8R8A8_UNORM:
		case RENDER_FORMAT_B8G8R8X8_UNORM:
		case RENDER_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case RENDER_FORMAT_B8G8R8A8_TYPELESS:
		case RENDER_FORMAT_B8G8R8A8_UNORM_SRGB:
		case RENDER_FORMAT_B8G8R8X8_TYPELESS:
		case RENDER_FORMAT_B8G8R8X8_UNORM_SRGB:
			return 32;

		case RENDER_FORMAT_R8G8_TYPELESS:
		case RENDER_FORMAT_R8G8_UNORM:
		case RENDER_FORMAT_R8G8_UINT:
		case RENDER_FORMAT_R8G8_SNORM:
		case RENDER_FORMAT_R8G8_SINT:
		case RENDER_FORMAT_R16_TYPELESS:
		case RENDER_FORMAT_R16_FLOAT:
		case RENDER_FORMAT_D16_UNORM:
		case RENDER_FORMAT_R16_UNORM:
		case RENDER_FORMAT_R16_UINT:
		case RENDER_FORMAT_R16_SNORM:
		case RENDER_FORMAT_R16_SINT:
		case RENDER_FORMAT_B5G6R5_UNORM:
		case RENDER_FORMAT_B5G5R5A1_UNORM:

#ifdef DXGI_1_2_FORMATS
		case RENDER_FORMAT_B4G4R4A4_UNORM:
#endif
			return 16;

		case RENDER_FORMAT_R8_TYPELESS:
		case RENDER_FORMAT_R8_UNORM:
		case RENDER_FORMAT_R8_UINT:
		case RENDER_FORMAT_R8_SNORM:
		case RENDER_FORMAT_R8_SINT:
		case RENDER_FORMAT_A8_UNORM:
			return 8;

		case RENDER_FORMAT_R1_UNORM:
			return 1;

		case RENDER_FORMAT_BC1_TYPELESS:
		case RENDER_FORMAT_BC1_UNORM:
		case RENDER_FORMAT_BC1_UNORM_SRGB:
		case RENDER_FORMAT_BC4_TYPELESS:
		case RENDER_FORMAT_BC4_UNORM:
		case RENDER_FORMAT_BC4_SNORM:
			return 4;

		case RENDER_FORMAT_BC2_TYPELESS:
		case RENDER_FORMAT_BC2_UNORM:
		case RENDER_FORMAT_BC2_UNORM_SRGB:
		case RENDER_FORMAT_BC3_TYPELESS:
		case RENDER_FORMAT_BC3_UNORM:
		case RENDER_FORMAT_BC3_UNORM_SRGB:
		case RENDER_FORMAT_BC5_TYPELESS:
		case RENDER_FORMAT_BC5_UNORM:
		case RENDER_FORMAT_BC5_SNORM:
		case RENDER_FORMAT_BC6H_TYPELESS:
		case RENDER_FORMAT_BC6H_UF16:
		case RENDER_FORMAT_BC6H_SF16:
		case RENDER_FORMAT_BC7_TYPELESS:
		case RENDER_FORMAT_BC7_UNORM:
		case RENDER_FORMAT_BC7_UNORM_SRGB:
			return 8;

		default:
			ToyGE_ASSERT_FAIL("Unexpected Render Format");
			return 0;
		}
	}

	bool IsCompress(RenderFormat fmt)
	{
		switch (fmt)
		{
		case RENDER_FORMAT_BC1_TYPELESS:
		case RENDER_FORMAT_BC1_UNORM:
		case RENDER_FORMAT_BC1_UNORM_SRGB:
		case RENDER_FORMAT_BC4_TYPELESS:
		case RENDER_FORMAT_BC4_UNORM:
		case RENDER_FORMAT_BC4_SNORM:
		case RENDER_FORMAT_BC2_TYPELESS:
		case RENDER_FORMAT_BC2_UNORM:
		case RENDER_FORMAT_BC2_UNORM_SRGB:
		case RENDER_FORMAT_BC3_TYPELESS:
		case RENDER_FORMAT_BC3_UNORM:
		case RENDER_FORMAT_BC3_UNORM_SRGB:
		case RENDER_FORMAT_BC5_TYPELESS:
		case RENDER_FORMAT_BC5_UNORM:
		case RENDER_FORMAT_BC5_SNORM:
		case RENDER_FORMAT_BC6H_TYPELESS:
		case RENDER_FORMAT_BC6H_UF16:
		case RENDER_FORMAT_BC6H_SF16:
		case RENDER_FORMAT_BC7_TYPELESS:
		case RENDER_FORMAT_BC7_UNORM:
		case RENDER_FORMAT_BC7_UNORM_SRGB:
			return true;

		default:
			return false;
		}
	}

	void ComputeMipsSize(
		int32_t width,
		int32_t heigth,
		int32_t depth,
		std::vector<int3> & outMipsSize)
	{
		outMipsSize.clear();

		int32_t x = std::max<int32_t>(1, width);
		int32_t y = std::max<int32_t>(1, heigth);
		int32_t z = std::max<int32_t>(1, depth);

		// Mip 0
		outMipsSize.push_back(int3(x, y, z));

		// Loop until x=y=z=1
		while ((x != 1) || (y != 1) || (z != 1))
		{
			x = std::max<int32_t>(1, x >> 1);
			y = std::max<int32_t>(1, y >> 1);
			z = std::max<int32_t>(1, z >> 1);

			outMipsSize.push_back(int3(x, y, z));
		}
	}

	static float GausscianFactor(float x)
	{
		return exp(x * x * -0.5f);
	}

	const std::vector<float> & GetGaussTable(int32_t numSamples)
	{
		static std::vector<std::vector<float>> gaussTables;
		
		if (numSamples >= (int32_t)gaussTables.size())
		{
			gaussTables.resize(numSamples + 1);
		}

		auto & table = gaussTables[numSamples];

		if (table.size() <= 0)
		{
			float sum = 0.0f;

			float step = 6.0f / (float)(numSamples - 1);
			float start = -3.0f;

			for (int32_t i = 0; i < numSamples; ++i)
			{
				float x = start + (float)i * step;
				float factor = GausscianFactor(x);
				sum += factor;
				table.push_back(factor);
			}

			// Normalize
			for (int32_t i = 0; i < numSamples; ++i)
			{
				table[i] /= sum;
			}
		}

		return table;
	}

	RenderViewport GetTextureQuadViewport(const Ptr<Texture> & tex)
	{
		RenderViewport vp;
		vp.width = (float)tex->GetDesc().width;
		vp.height = (float)tex->GetDesc().height;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		return vp;
	}

	RenderViewport GetQuadViewport()
	{
		RenderViewport vp;
		vp.width = (float)Global::GetWindow()->Width();
		vp.height = (float)Global::GetWindow()->Height();
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		return vp;
	}

	Ptr<Texture> HeightToBumpTex(const Ptr<Texture> & heightTex, float scale)
	{
		auto bumpTexDesc = heightTex->GetDesc();
		bumpTexDesc.format = RENDER_FORMAT_R8G8B8A8_UNORM;
		bumpTexDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
		bumpTexDesc.mipLevels = 0;
		auto bumpTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		bumpTex->SetDesc(bumpTexDesc);
		bumpTex->Init();

		auto ps = Shader::FindOrCreate<HeightToBumpPS>();

		ps->SetScalar("texSize", heightTex->GetTexSize());
		ps->SetScalar("scale", scale);

		ps->SetSRV("heightTex", heightTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ bumpTex->GetRenderTargetView(0, 0, 1) });

		bumpTex->GenerateMips();

		return bumpTex;
	}

	const std::vector<Ptr<VertexBuffer>> & GetQuadVBs()
	{
		static std::vector<Ptr<VertexBuffer>> quadVBs;
		if (quadVBs.size() == 0)
		{
			// Postions
			{
				auto quadPositionVB = Global::GetRenderEngine()->GetRenderFactory()->CreateVertexBuffer();

				RenderBufferDesc desc;
				desc.bindFlag = BUFFER_BIND_VERTEX;
				desc.cpuAccess = 0;
				desc.elementSize = sizeof(float2);
				desc.numElements = 4;
				desc.bStructured = false;
				quadPositionVB->SetDesc(desc);

				VertexElementDesc elementDesc;
				elementDesc.bytesOffset = 0;
				elementDesc.bytesSize = sizeof(float2);
				elementDesc.format = RENDER_FORMAT_R32G32_FLOAT;
				elementDesc.name = "POSITION";
				elementDesc.index = 0;
				elementDesc.instanceDataRate = 0;
				quadPositionVB->SetElementsDesc({ elementDesc });

				float2 quadPositions[4] =
				{
					{ -1.0f,  1.0f },
					{ 1.0f,  1.0f },
					{ -1.0f, -1.0f },
					{ 1.0f, -1.0f }
				};
				quadPositionVB->Init(quadPositions);
				quadVBs.push_back(quadPositionVB);
			}

			// UVs
			{
				auto quadUVVB = Global::GetRenderEngine()->GetRenderFactory()->CreateVertexBuffer();

				RenderBufferDesc desc;
				desc.bindFlag = BUFFER_BIND_VERTEX;
				desc.cpuAccess = CPU_ACCESS_WRITE;
				desc.elementSize = sizeof(float2);
				desc.numElements = 4;
				desc.bStructured = false;
				quadUVVB->SetDesc(desc);

				VertexElementDesc elementDesc;
				elementDesc.bytesOffset = 0;
				elementDesc.bytesSize = sizeof(float2);
				elementDesc.format = RENDER_FORMAT_R32G32_FLOAT;
				elementDesc.name = "TEXCOORD";
				elementDesc.index = 0;
				elementDesc.instanceDataRate = 0;
				quadUVVB->SetElementsDesc({ elementDesc });

				quadUVVB->Init(nullptr);
				quadVBs.push_back(quadUVVB);
			}
		}

		return quadVBs;
	}

	const Ptr<RenderBuffer> & GetQuadIB()
	{
		static Ptr<RenderBuffer> quadIB;

		if (!quadIB)
		{
			quadIB = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();

			RenderBufferDesc desc;
			desc.bindFlag = BUFFER_BIND_INDEX;
			desc.cpuAccess = 0;
			desc.elementSize = sizeof(uint32_t);
			desc.numElements = 6;
			desc.bStructured = false;
			quadIB->SetDesc(desc);

			uint32_t quadIndices[] =
			{
				0, 1, 2,
				1, 3, 2
			};
			quadIB->Init(quadIndices);
		}

		return quadIB;
	}

	void DrawQuad(
		const std::vector< Ptr<class RenderTargetView> > & rtvs,
		float topLeftX,
		float topLeftY,
		float width,
		float height,
		float topLeftU,
		float topLeftV,
		float uvWidth,
		float uvHeight,
		const Ptr<class DepthStencilView> & dsv)
	{
		/*if (rtvs.size() == 0)
			return;*/

		auto & quadVBs = GetQuadVBs();
		auto & quadIB = GetQuadIB();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		// Viewport
		if (width == 0.0f)
		{
			if (rtvs.size() > 0)
			{
				auto tex = rtvs[0]->GetResource()->Cast<Texture>();
				auto & mipSize = tex->GetMipSize(rtvs[0]->Cast<TextureRenderTargetView>()->mipLevel);
				width = (float)mipSize.x();
			}
			else if (dsv)
			{
				auto tex = dsv->GetResource()->Cast<Texture>();
				auto & mipSize = tex->GetMipSize(dsv->Cast<TextureDepthStencilView>()->mipLevel);
				width = (float)mipSize.x();
			}
		}
		if (height == 0.0f)
		{
			if (rtvs.size() > 0)
			{
				auto tex = rtvs[0]->GetResource()->Cast<Texture>();
				auto & mipSize = tex->GetMipSize(rtvs[0]->Cast<TextureRenderTargetView>()->mipLevel);
				height = (float)mipSize.y();
			}
			else if (dsv)
			{
				auto tex = dsv->GetResource()->Cast<Texture>();
				auto & mipSize = tex->GetMipSize(dsv->Cast<TextureDepthStencilView>()->mipLevel);
				height = (float)mipSize.y();
			}
		}

		RenderViewport vp;
		vp.topLeftX = static_cast<float>(topLeftX);
		vp.topLeftY = static_cast<float>(topLeftY);
		vp.width = static_cast<float>(width);
		vp.height = static_cast<float>(height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		// Update uvs
		float2 uvMap[4];
		uvMap[0] = float2(topLeftU,				topLeftV);
		uvMap[1] = float2(topLeftU + uvWidth,	topLeftV);
		uvMap[2] = float2(topLeftU,				topLeftV + uvHeight);
		uvMap[3] = float2(topLeftU + uvWidth,	topLeftV + uvHeight);

		auto uvBufMappedData = quadVBs[1]->Map(MAP_WRITE_DISCARD);
		memcpy(uvBufMappedData.pData, uvMap, sizeof(float2) * 4);
		quadVBs[1]->UnMap();

		// Set vbs, ib
		rc->SetVertexBuffer(quadVBs);
		rc->SetIndexBuffer(quadIB);
		rc->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set rtv
		rc->SetRenderTargets(rtvs);

		// Set dsv
		rc->SetDepthStencil(dsv);
		if (!dsv)
			rc->SetDepthStencilState(DepthStencilStateTemplate<false>::Get());

		// Bind shader
		auto drawQuadVS = Shader::FindOrCreate<DrawQuadVS>();
		drawQuadVS->Flush();

		// Draw
		rc->DrawIndexed(0, 0);

		if (!dsv)
			rc->SetDepthStencilState(nullptr);
	}

	void RenderToVolumeTexture(const Ptr<Texture> & volumeTexture)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto & quadVBs = GetQuadVBs();
		auto & quadIB = GetQuadIB();

		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(volumeTexture->GetDesc().width);
		vp.height = static_cast<float>(volumeTexture->GetDesc().height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		float2 uvMap[4];
		uvMap[0] = float2(0.0f, 0.0f);
		uvMap[1] = float2(1.0f, 0.0f);
		uvMap[2] = float2(0.0f, 1.0f);
		uvMap[3] = float2(1.0f, 1.0f);

		auto uvBufMappedData = quadVBs[1]->Map(MAP_WRITE_DISCARD);
		memcpy(uvBufMappedData.pData, uvMap, sizeof(float2) * 4);
		quadVBs[1]->UnMap();

		// Set vbs, ib
		rc->SetVertexBuffer(quadVBs);
		rc->SetIndexBuffer(quadIB);
		rc->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set rtv
		rc->SetRenderTargets({ volumeTexture->GetRenderTargetView(0, 0, volumeTexture->GetDesc().depth) });

		// Set dsv
		rc->SetDepthStencilState(DepthStencilStateTemplate<false>::Get());

		// Bind shader
		auto vs = Shader::FindOrCreate<RenderToVolumeTextureVS>();
		auto gs = Shader::FindOrCreate<RenderToVolumeTextureGS>();
		vs->Flush();
		gs->Flush();

		// Draw
		rc->DrawIndexedInstanced(quadIB->GetDesc().numElements, volumeTexture->GetDesc().depth, 0, 0, 0);

		rc->SetDepthStencilState(nullptr);
		rc->ResetShader(SHADER_GS);
	}

	void Fill(
		const float4 & color,
		const std::vector< Ptr<class RenderTargetView> > & rtvs,
		float topLeftX,
		float topLeftY,
		float width,
		float height,
		float topLeftU,
		float topLeftV,
		float uvWidth,
		float uvHeight,
		const Ptr<class DepthStencilView> & dsv)
	{
		auto ps = Shader::FindOrCreate<FillPS>();
		ps->SetScalar("fillColor", color);
		ps->Flush();

		DrawQuad(rtvs, topLeftX, topLeftY, width, height, topLeftU, topLeftV, uvWidth, uvHeight, dsv);
	}

	//IMPLEMENT_SHADER(TransformPS, SHADER_PS, "TransformPS", "TransformPS");

	void Transform(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		const Vector<ColorWriteMask, 4> & colorWriteMask,
		const float4 & srcRect,
		const float4 & dstRect,
		const Ptr<class Sampler> & sampler,
		const Ptr<DepthStencilView> & dsv )
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		std::map<String, String> macros;
		for (int32_t i = 0; i < 4; ++i)
		{
			auto & writeMask = colorWriteMask[i];
			String writeChannel = std::to_string(static_cast<uint32_t>(std::log2(static_cast<uint32_t>(writeMask))));
			macros["COLOR_CHANNEL_" + std::to_string(i)] = writeChannel;
		}

		auto transformPS = Shader::FindOrCreate<TransformPS>(macros);
		transformPS->SetSampler("transformSampler", sampler ? sampler : SamplerTemplate<>::Get());
		transformPS->SetSRV("srcTex", src);
		transformPS->Flush();

		auto srcTex = src->GetResource()->Cast<Texture>();
		float topLeftU = srcRect.x() / static_cast<float>(srcTex->GetDesc().width);
		float topLeftV = srcRect.y() / static_cast<float>(srcTex->GetDesc().height);
		float uvWidth  = srcRect.z() / static_cast<float>(srcTex->GetDesc().width);
		float uvHeight = srcRect.w() / static_cast<float>(srcTex->GetDesc().height);

		if (uvWidth == 0.0f)
			uvWidth = 1.0f;
		if (uvHeight == 0.0f)
			uvHeight = 1.0f;

		DrawQuad({ dst },
			dstRect.x(),
			dstRect.y(),
			dstRect.z(),
			dstRect.w(),
			topLeftU,
			topLeftV,
			uvWidth,
			uvHeight, 
			dsv);
	}

	void TextureFilter(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		const std::vector<float2> & uvOffsets,
		const std::vector<float> & weights,
		const Ptr<class Sampler> & sampler)
	{
		ToyGE_ASSERT(uvOffsets.size() == weights.size());
		ToyGE_ASSERT(src);
		ToyGE_ASSERT(dst);

		int32_t numSamples = (int32_t)uvOffsets.size();
		if (numSamples <= 0)
			return;

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetViewport(GetTextureQuadViewport(dst->GetResource()->Cast<Texture>()));

		auto filterVS = Shader::FindOrCreate<FilterVS>({ { "NUM_SAMPLES", std::to_string(numSamples) } });
		auto filterPS = Shader::FindOrCreate<FilterPS>({ { "NUM_SAMPLES", std::to_string(numSamples) } });

		filterVS->SetScalar("samplesOffsets", &uvOffsets[0], (int32_t)(sizeof(uvOffsets[0]) * uvOffsets.size()));
		filterVS->Flush();

		filterPS->SetScalar("samplesWeights", &weights[0], (int32_t)(sizeof(weights[0]) * weights.size()));
		filterPS->SetSRV("filterTex", src);
		filterPS->SetSampler("filterSampler", sampler ? sampler : SamplerTemplate<>::Get());
		filterPS->Flush();

		rc->SetRenderTargets({ dst });

		rc->SetDepthStencil(nullptr);
		rc->SetDepthStencilState(DepthStencilStateTemplate<false>::Get());

		rc->SetVertexBuffer({ GetQuadVBs()[0] });
		rc->SetIndexBuffer(GetQuadIB());
		rc->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		rc->DrawIndexed(0, 0);
	}

	void BilateralBlur(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		const Ptr<ShaderResourceView> & depthTex,
		const std::vector<float> & weights,
		float depthDiffThreshold)
	{
		auto srcTex = src->GetResource()->Cast<Texture>();

		auto tmpTexDesc = dst->GetResource()->Cast<Texture>()->GetDesc();
		auto tmpTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, tmpTexDesc });

		{
			auto ps = Shader::FindOrCreate<BilateralBlurXPS>();
			ps->SetScalar("bilateralBlurWeights", &weights[0], (int32_t)(sizeof(float) * weights.size()));
			ps->SetScalar("texSize", srcTex->GetTexSize());
			ps->SetScalar("depthDiffThreshold", depthDiffThreshold);

			ps->SetSRV("bilateralDepthTex", depthTex);
			ps->SetSRV("bilateralBlurInTex", src);

			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			ps->Flush();

			DrawQuad({ tmpTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });
		}

		{
			auto ps = Shader::FindOrCreate<BilateralBlurYPS>();
			ps->SetScalar("bilateralBlurWeights", &weights[0], (int32_t)(sizeof(float) * weights.size()));
			ps->SetScalar("texSize", srcTex->GetTexSize());
			ps->SetScalar("depthDiffThreshold", depthDiffThreshold);

			ps->SetSRV("bilateralDepthTex", depthTex);
			ps->SetSRV("bilateralBlurInTex", tmpTexRef->Get()->Cast<Texture>()->GetShaderResourceView());

			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			ps->Flush();

			DrawQuad({ dst });
		}
	}

	void BilateralUpSampling(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		const Ptr<ShaderResourceView> & lowResDepthTex,
		const Ptr<ShaderResourceView> & highResDepthTex,
		float depthDiffThreshold)
	{
		auto srcTex = src->GetResource()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<BilateralUpSamplingPS>();
		ps->SetScalar("texSize", srcTex->GetTexSize());
		ps->SetScalar("depthDiffThreshold", depthDiffThreshold);

		ps->SetSRV("lowResDepthTex", lowResDepthTex);
		ps->SetSRV("highResDepthTex", highResDepthTex);
		ps->SetSRV("upSamplingInTex", src);

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ dst });
	}

	void LinearizeDepth(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target)
	{
		auto ps = Shader::FindOrCreate<LinearizeDepthPS>();

		view->BindShaderParams(ps);
		ps->SetSRV("depthTex", depth);
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ target });
	}
}
