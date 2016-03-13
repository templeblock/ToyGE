#include "ToyGE\RenderEngine\FFT.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"

namespace ToyGE
{
	void FFT::FFT2DRadix2(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		bool bInverse,
		bool bIFFTScale)
	{
		auto mipSize = src->GetMipSize(srcMipLevel);

		TextureDesc texDesc;
		texDesc.width = mipSize.x();
		texDesc.height = mipSize.y();
		texDesc.depth = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		texDesc.mipLevels = 1;
		texDesc.arraySize = 1;
		texDesc.bCube = false;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		PooledTextureRef bfSrcRef;
		Ptr<Texture> bfSrc;
		int32_t bfSrcMipLevel = 0;
		int32_t bfSrcArrayOffset = 0;
		/*if (!bInverse)
		{
			bfSrcRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
			bfSrc = bfSrcRef->Get()->Cast<Texture>();
			FFT::FFTSetup(src, srcMipLevel, srcArrayOffset, bfSrc, 0, 0);
		}
		else*/
		{
			bfSrc = src;
			bfSrcMipLevel = srcMipLevel;
			bfSrcArrayOffset = srcArrayOffset;
		}

		PooledTextureRef bfDstRef;
		Ptr<Texture> bfDst;
		int32_t bfDstMipLevel = 0;
		int32_t bfDstArrayOffset = 0;
		if (dst->GetDesc().bindFlag & TEXTURE_BIND_UNORDERED_ACCESS && dst->GetDesc().format == RENDER_FORMAT_R32G32_FLOAT)
		{
			bfDst = dst;
			bfDstMipLevel = dstMipLevel;
			bfDstArrayOffset = dstArrayOffset;
		}
		else
		{
			bfDstRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
			bfDst = bfDstRef->Get()->Cast<Texture>();
		}

		
		std::map<String, String> macros;

		macros["FFT_2D"] = "";
		if (bInverse)
		{
			macros["FFT_INVERSE"] = "";
			if (bIFFTScale)
				macros["FFT_INVERSE_SCALE"] = "";
		}
		static int fftGroupSize = 64;
		macros["FFT_GROUP_SIZE"] = std::to_string(fftGroupSize);

		auto fftXCS = Shader::FindOrCreate<FFTRadix2_2D_XCS>(macros);
		auto fftYCS = Shader::FindOrCreate<FFTRadix2_2D_YCS>(macros);

		auto ifftScalePS = Shader::FindOrCreate<IFFTScalePS>();

		float4 dataSize = float4((float)mipSize.x(), (float)mipSize.y(), 1.0f / (float)mipSize.x(), 1.0f / (float)mipSize.y());

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		//Butterfly X
		{
			uint32_t numGroups = std::max<uint32_t>(1, texDesc.width / fftGroupSize / 2);

			uint32_t bfLen = 2;
			while (bfLen <= static_cast<uint32_t>(texDesc.width))
			{
				fftXCS->SetScalar("dataSize", dataSize);
				fftXCS->SetScalar("butterflyLength", bfLen);

				fftXCS->SetSRV("srcTex", bfSrc->GetShaderResourceView(bfSrcMipLevel, 1, bfSrcArrayOffset, 1));
				fftXCS->SetUAV("dstTex", bfDst->GetUnorderedAccessView(bfDstMipLevel, bfDstArrayOffset, 1));

				fftXCS->Flush();

				rc->Compute(numGroups, texDesc.height, 1);

				std::swap(bfSrc, bfDst);
				std::swap(bfSrcMipLevel, bfDstMipLevel);
				std::swap(bfSrcArrayOffset, bfDstArrayOffset);

				bfLen = bfLen << 1;
			}
			rc->ResetShader(SHADER_CS);

			if (bInverse && bIFFTScale)
			{
				float ifftScale = 1.0f / (float)texDesc.width;
				ifftScalePS->SetScalar("ifftScale", ifftScale);
				ifftScalePS->SetSRV("srcTex", bfSrc->GetShaderResourceView(bfSrcMipLevel, 1, bfSrcArrayOffset, 1));
				ifftScalePS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
				ifftScalePS->Flush();

				DrawQuad({ bfDst->GetRenderTargetView(dstMipLevel, dstArrayOffset, 1) });

				std::swap(bfSrc, bfDst);
				std::swap(bfSrcMipLevel, bfDstMipLevel);
				std::swap(bfSrcArrayOffset, bfDstArrayOffset);
			}
		}

		//Butterfly Y
		{
			uint32_t numGroups = std::max<uint32_t>(1, texDesc.height / fftGroupSize / 2);

			uint32_t bfLen = 2;
			while (bfLen <= static_cast<uint32_t>(texDesc.height))
			{
				fftYCS->SetScalar("dataSize", dataSize);
				fftYCS->SetScalar("butterflyLength", bfLen);

				fftYCS->SetSRV("srcTex", bfSrc->GetShaderResourceView(bfSrcMipLevel, 1, bfSrcArrayOffset, 1));
				fftYCS->SetUAV("dstTex", bfDst->GetUnorderedAccessView(bfDstMipLevel, bfDstArrayOffset, 1));

				fftYCS->Flush();

				rc->Compute(numGroups, texDesc.width, 1);

				std::swap(bfSrc, bfDst);
				std::swap(bfSrcMipLevel, bfDstMipLevel);
				std::swap(bfSrcArrayOffset, bfDstArrayOffset);

				bfLen = bfLen << 1;
			}
			rc->ResetShader(SHADER_CS);

			if (bInverse && bIFFTScale)
			{
				float ifftScale = 1.0f / texDesc.height;
				ifftScalePS->SetScalar("ifftScale", ifftScale);
				ifftScalePS->SetSRV("srcTex", bfSrc->GetShaderResourceView(bfSrcMipLevel, 1, bfSrcArrayOffset, 1));
				ifftScalePS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
				ifftScalePS->Flush();

				DrawQuad({ bfDst->GetRenderTargetView(dstMipLevel, dstArrayOffset, 1) });

				std::swap(bfSrc, bfDst);
				std::swap(bfSrcMipLevel, bfDstMipLevel);
				std::swap(bfSrcArrayOffset, bfDstArrayOffset);
			}
		}

		//Clear
		if (bfSrc != dst) //dst is result
		{
			bfSrc->CopyTo(dst, dstMipLevel, dstArrayOffset, 0, 0, 0, 0, 0);
		}
	}

	void FFT::FFT2DRadix8(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		bool bInverse,
		bool bIFFTScale)
	{
		auto mipSize = src->GetMipSize(srcMipLevel);

		TextureDesc texDesc;
		texDesc.width = mipSize.x();
		texDesc.height = mipSize.y();
		texDesc.depth = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		texDesc.mipLevels = 1;
		texDesc.arraySize = 1;
		texDesc.bCube = false;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		PooledTextureRef bfSrcRef;
		Ptr<Texture> bfSrc;
		int32_t bfSrcMipLevel = 0;
		int32_t bfSrcArrayOffset = 0;
		//if (!bInverse)
		//{
		//	bfSrcRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		//	bfSrc = bfSrcRef->Get()->Cast<Texture>();
		//	FFT::FFTSetup(src, srcMipLevel, srcArrayOffset, bfSrc, 0, 0);
		//}
		//else
		{
			bfSrc = src;
			bfSrcMipLevel = srcMipLevel;
			bfSrcArrayOffset = srcArrayOffset;
		}

		PooledTextureRef bfDstRef;
		Ptr<Texture> bfDst;
		int32_t bfDstMipLevel = 0;
		int32_t bfDstArrayOffset = 0;
		if (dst->GetDesc().bindFlag & TEXTURE_BIND_UNORDERED_ACCESS && dst->GetDesc().format == RENDER_FORMAT_R32G32_FLOAT)
		{
			bfDst = dst;
			bfDstMipLevel = dstMipLevel;
			bfDstArrayOffset = dstArrayOffset;
		}
		else
		{
			bfDstRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
			bfDst = bfDstRef->Get()->Cast<Texture>();
		}


		std::map<String, String> macros;

		macros["FFT_2D"] = "";
		if (bInverse)
		{
			macros["FFT_INVERSE"] = "";
			if(bIFFTScale)
				macros["FFT_INVERSE_SCALE"] = "";
		}
		static int fftGroupSize = 64;
		macros["FFT_GROUP_SIZE"] = std::to_string(fftGroupSize);

		auto fftXCS = Shader::FindOrCreate<FFTRadix8_2D_XCS>(macros);
		auto fftXFinalCS = Shader::FindOrCreate<FFTRadix8_2D_X_FinalCS>(macros);
		auto fftYCS = Shader::FindOrCreate<FFTRadix8_2D_YCS>(macros);
		auto fftYFinalCS = Shader::FindOrCreate<FFTRadix8_2D_Y_FinalCS>(macros);

		auto ifftScalePS = Shader::FindOrCreate<IFFTScalePS>();

		float4 dataSize = float4((float)mipSize.x(), (float)mipSize.y(), 1.0f / (float)mipSize.x(), 1.0f / (float)mipSize.y());

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		//FFT X
		{
			uint32_t numGroups = texDesc.width / fftGroupSize / 8;

			uint32_t threadCount = texDesc.width / 8;
			uint32_t ostride = threadCount;

			uint32_t istride = threadCount;
			float phaseBase = -PI2 / texDesc.width;
			if (bInverse)
				phaseBase *= -1.0f;
			while (istride > 0)
			{
				if (istride > 1)
				{
					fftXCS->SetScalar("dataSize", dataSize);
					fftXCS->SetScalar("threadCount", threadCount);
					fftXCS->SetScalar("ostride", ostride);
					fftXCS->SetScalar("istride", istride);
					fftXCS->SetScalar("phaseBase", phaseBase);

					fftXCS->SetSRV("srcTex", bfSrc->GetShaderResourceView(bfSrcMipLevel, 1, bfSrcArrayOffset, 1));
					fftXCS->SetUAV("dstTex", bfDst->GetUnorderedAccessView(bfDstMipLevel, bfDstArrayOffset, 1));

					fftXCS->Flush();

					rc->Compute(numGroups, texDesc.height, 1);
				}
				else
				{
					fftXFinalCS->SetScalar("dataSize", dataSize);
					fftXFinalCS->SetScalar("threadCount", threadCount);
					fftXFinalCS->SetScalar("ostride", ostride);
					fftXFinalCS->SetScalar("istride", istride);

					fftXFinalCS->SetSRV("srcTex", bfSrc->GetShaderResourceView(bfSrcMipLevel, 1, bfSrcArrayOffset, 1));
					fftXFinalCS->SetUAV("dstTex", bfDst->GetUnorderedAccessView(bfDstMipLevel, bfDstArrayOffset, 1));

					fftXFinalCS->Flush();

					rc->Compute(numGroups, texDesc.height, 1);
				}

				std::swap(bfSrc, bfDst);
				std::swap(bfSrcMipLevel, bfDstMipLevel);
				std::swap(bfSrcArrayOffset, bfDstArrayOffset);

				istride /= 8;
				phaseBase *= 8.0f;
			}

		}

		//FFT Y
		{
			uint32_t numGroups = texDesc.height / fftGroupSize / 8;

			uint32_t threadCount = texDesc.height / 8;
			uint32_t ostride = threadCount;

			uint32_t istride = threadCount;
			float phaseBase = -PI2 / texDesc.height;
			if (bInverse)
				phaseBase *= -1.0f;
			while (istride > 0)
			{
				if (istride > 1)
				{
					fftYCS->SetScalar("dataSize", dataSize);
					fftYCS->SetScalar("threadCount", threadCount);
					fftYCS->SetScalar("ostride", ostride);
					fftYCS->SetScalar("istride", istride);
					fftYCS->SetScalar("phaseBase", phaseBase);

					fftYCS->SetSRV("srcTex", bfSrc->GetShaderResourceView(bfSrcMipLevel, 1, bfSrcArrayOffset, 1));
					fftYCS->SetUAV("dstTex", bfDst->GetUnorderedAccessView(bfDstMipLevel, bfDstArrayOffset, 1));

					fftYCS->Flush();

					rc->Compute(numGroups, texDesc.width, 1);
				}
				else
				{
					fftYFinalCS->SetScalar("dataSize", dataSize);
					fftYFinalCS->SetScalar("threadCount", threadCount);
					fftYFinalCS->SetScalar("ostride", ostride);
					fftYFinalCS->SetScalar("istride", istride);

					fftYFinalCS->SetSRV("srcTex", bfSrc->GetShaderResourceView(bfSrcMipLevel, 1, bfSrcArrayOffset, 1));
					fftYFinalCS->SetUAV("dstTex", bfDst->GetUnorderedAccessView(bfDstMipLevel, bfDstArrayOffset, 1));

					fftYFinalCS->Flush();

					rc->Compute(numGroups, texDesc.width, 1);
				}

				std::swap(bfSrc, bfDst);
				std::swap(bfSrcMipLevel, bfDstMipLevel);
				std::swap(bfSrcArrayOffset, bfDstArrayOffset);

				istride /= 8;
				phaseBase *= 8.0f;
			}

		}

		rc->ResetShader(SHADER_CS);

		//Clear
		if (bfSrc != dst) //dst is result
		{
			bfSrc->CopyTo(dst, dstMipLevel, dstArrayOffset, 0, 0, 0, 0, 0);
		}
	}

	void FFT::FFTSetup(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset)
	{
		auto ps = Shader::FindOrCreate<FFTSetupPS>();

		ps->SetSRV("srcTex", src->GetShaderResourceView(srcMipLevel, 1, srcArrayOffset, 1));
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->Flush();

		DrawQuad({ dst->GetRenderTargetView(dstMipLevel, dstArrayOffset, 1) });
	}
}