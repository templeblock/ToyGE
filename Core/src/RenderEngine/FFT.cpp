#include "ToyGE\RenderEngine\FFT.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	void FFT::FFT2DRadix2(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		bool bInverse)
	{
		if (src->GetMipSize(srcMipLevel) != dst->GetMipSize(dstMipLevel)
			|| src->Desc().format != RENDER_FORMAT_R32G32_FLOAT
			|| dst->Desc().format != RENDER_FORMAT_R32G32_FLOAT)
			return;

		//Prepare src&dst textures for butterfly
		auto mipSize = src->GetMipSize(srcMipLevel);
		auto texDesc = src->Desc();
		texDesc.width = std::get<0>(mipSize);
		texDesc.height = std::get<1>(mipSize);
		texDesc.arraySize = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;

		auto bfSrcTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		src->CopyTo(bfSrcTex, 0, 0, 0, 0, 0, srcMipLevel, srcArrayOffset);

		Ptr<Texture> bfDstTex;
		int32_t bfDstMipLevel = 0;
		int32_t bfDstArrayOffset = 0;
		if (dst->Desc().bindFlag & TEXTURE_BIND_UNORDERED_ACCESS)
		{
			bfDstTex = dst;
			bfDstMipLevel = dstMipLevel;
			bfDstArrayOffset = dstArrayOffset;
		}
		else
		{
			bfDstTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		}

		//FX
		auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"FFT.xml");
		fx->AddExtraMacro("FFT_2D", "");
		if (bInverse)
			fx->AddExtraMacro("FFT_INVERSE", "");
		else
			fx->RemoveExtraMacro("FFT_INVERSE");
		fx->UpdateData();
		auto fxUtil = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"FFTUtil.xml");

		fx->VariableByName("dataSize")->AsScalar()->SetValue(&src->GetTexSize());

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		const int32_t groupSize = std::stoi(fx->MacroByName("FFT_GROUP_SIZE").value);

		//Butterfly X
		{
			uint32_t numGroups = std::max<uint32_t>(1, texDesc.width / groupSize / 2);

			uint32_t bfLen = 2;
			while (bfLen <= static_cast<uint32_t>(texDesc.width))
			{
				fx->VariableByName("butterflyLength")->AsScalar()->SetValue(&bfLen);

				fx->VariableByName("srcTex")->AsShaderResource()->SetValue(bfSrcTex->CreateTextureView());
				fx->VariableByName("dstTex")->AsUAV()->SetValue(bfDstTex->CreateTextureView(bfDstMipLevel, 1, bfDstArrayOffset, 1));

				fx->TechniqueByName("FFTRadix2_2D_X")->PassByIndex(0)->Bind();
				rc->Compute(numGroups, texDesc.height, 1);
				fx->TechniqueByName("FFTRadix2_2D_X")->PassByIndex(0)->UnBind();

				std::swap(bfSrcTex, bfDstTex);

				bfLen = bfLen << 1;
			}

			if (bInverse)
			{
				float ifftScale = 1.0f / texDesc.width;
				fxUtil->VariableByName("ifftScale")->AsScalar()->SetValue(&ifftScale);
				fxUtil->VariableByName("srcTex")->AsShaderResource()->SetValue(bfSrcTex->CreateTextureView());
				rc->SetRenderTargets({ bfDstTex->CreateTextureView() }, 0);
				RenderQuad(fxUtil->TechniqueByName("IFFTScale"));

				std::swap(bfSrcTex, bfDstTex);

				rc->SetRenderTargets({}, 0);
			}
		}

		//Butterfly Y
		{
			uint32_t numGroups = std::max<uint32_t>(1, texDesc.height / groupSize / 2);

			uint32_t bfLen = 2;
			while (bfLen <= static_cast<uint32_t>(texDesc.height))
			{
				fx->VariableByName("butterflyLength")->AsScalar()->SetValue(&bfLen);

				fx->VariableByName("srcTex")->AsShaderResource()->SetValue(bfSrcTex->CreateTextureView());
				fx->VariableByName("dstTex")->AsUAV()->SetValue(bfDstTex->CreateTextureView(bfDstMipLevel, 1, bfDstArrayOffset, 1));

				fx->TechniqueByName("FFTRadix2_2D_Y")->PassByIndex(0)->Bind();
				rc->Compute(numGroups, texDesc.width, 1);
				fx->TechniqueByName("FFTRadix2_2D_Y")->PassByIndex(0)->UnBind();

				std::swap(bfSrcTex, bfDstTex);

				bfLen = bfLen << 1;
			}

			if (bInverse)
			{
				float ifftScale = 1.0f / texDesc.height;
				fxUtil->VariableByName("ifftScale")->AsScalar()->SetValue(&ifftScale);
				fxUtil->VariableByName("srcTex")->AsShaderResource()->SetValue(bfSrcTex->CreateTextureView());
				rc->SetRenderTargets({ bfDstTex->CreateTextureView() }, 0);
				RenderQuad(fxUtil->TechniqueByName("IFFTScale"));

				std::swap(bfSrcTex, bfDstTex);

				rc->SetRenderTargets({}, 0);
			}
		}

		//Clear
		if (bfSrcTex == dst) //dst is result
		{
			bfDstTex->Release();
		}
		else //dst is not result, copy from bfSrcTex
		{
			bfSrcTex->CopyTo(dst, dstMipLevel, dstArrayOffset, 0, 0, 0, 0, 0);
			if (bfDstTex != dst)
				bfDstTex->Release();
		}
	}

	void FFT::FFT2DRadix8(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		bool bInverse)
	{
		if (src->GetMipSize(srcMipLevel) != dst->GetMipSize(dstMipLevel)
			|| src->Desc().format != RENDER_FORMAT_R32G32_FLOAT
			|| dst->Desc().format != RENDER_FORMAT_R32G32_FLOAT)
			return;

		//Prepare src&dst textures for butterfly
		auto mipSize = src->GetMipSize(srcMipLevel);
		auto texDesc = src->Desc();
		texDesc.width = std::get<0>(mipSize);
		texDesc.height = std::get<1>(mipSize);
		texDesc.arraySize = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;

		auto bfSrcTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		src->CopyTo(bfSrcTex, 0, 0, 0, 0, 0, srcMipLevel, srcArrayOffset);

		Ptr<Texture> bfDstTex;
		int32_t bfDstMipLevel = 0;
		int32_t bfDstArrayOffset = 0;
		if (dst->Desc().bindFlag & TEXTURE_BIND_UNORDERED_ACCESS)
		{
			bfDstTex = dst;
			bfDstMipLevel = dstMipLevel;
			bfDstArrayOffset = dstArrayOffset;
		}
		else
		{
			bfDstTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		}

		//FX
		auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"FFT.xml");
		fx->AddExtraMacro("FFT_2D", "");
		if (bInverse)
			fx->AddExtraMacro("FFT_INVERSE", "");
		else
			fx->RemoveExtraMacro("FFT_INVERSE");
		fx->UpdateData();

		fx->VariableByName("dataSize")->AsScalar()->SetValue(&src->GetTexSize());

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		const int32_t groupSize = std::stoi(fx->MacroByName("FFT_GROUP_SIZE").value);

		//FFT X
		{
			uint32_t numGroups = texDesc.width / groupSize / 8;

			uint32_t threadCount = texDesc.width / 8;
			fx->VariableByName("threadCount")->AsScalar()->SetValue(&threadCount);
			uint32_t ostirde = threadCount;
			fx->VariableByName("ostride")->AsScalar()->SetValue(&ostirde);

			uint32_t istride = threadCount;
			float phaseBase = -XM_2PI / texDesc.width;
			if (bInverse)
				phaseBase *= -1.0f;
			while (istride > 0)
			{
				fx->VariableByName("istride")->AsScalar()->SetValue(&istride);
				fx->VariableByName("phaseBase")->AsScalar()->SetValue(&phaseBase);

				fx->VariableByName("srcTex")->AsShaderResource()->SetValue(bfSrcTex->CreateTextureView());
				fx->VariableByName("dstTex")->AsUAV()->SetValue(bfDstTex->CreateTextureView(bfDstMipLevel, 1, bfDstArrayOffset, 1));

				if (istride > 1)
				{
					fx->TechniqueByName("FFTRadix8_2D_X")->PassByIndex(0)->Bind();
					rc->Compute(numGroups, texDesc.height, 1);
					fx->TechniqueByName("FFTRadix8_2D_X")->PassByIndex(0)->UnBind();
				}
				else
				{
					fx->TechniqueByName("FFTRadix8_2D_X_Final")->PassByIndex(0)->Bind();
					rc->Compute(numGroups, texDesc.height, 1);
					fx->TechniqueByName("FFTRadix8_2D_X_Final")->PassByIndex(0)->UnBind();
				}

				std::swap(bfSrcTex, bfDstTex);

				istride /= 8;
				phaseBase *= 8.0f;
			}

		}

		//FFT Y
		{
			uint32_t numGroups = texDesc.height / groupSize / 8;

			uint32_t threadCount = texDesc.height / 8;
			fx->VariableByName("threadCount")->AsScalar()->SetValue(&threadCount);
			uint32_t ostirde = threadCount;
			fx->VariableByName("ostride")->AsScalar()->SetValue(&ostirde);

			uint32_t istride = threadCount;
			float phaseBase = -XM_2PI / texDesc.height;
			if (bInverse)
				phaseBase *= -1.0f;
			while (istride > 0)
			{
				fx->VariableByName("istride")->AsScalar()->SetValue(&istride);
				fx->VariableByName("phaseBase")->AsScalar()->SetValue(&phaseBase);

				fx->VariableByName("srcTex")->AsShaderResource()->SetValue(bfSrcTex->CreateTextureView());
				fx->VariableByName("dstTex")->AsUAV()->SetValue(bfDstTex->CreateTextureView(bfDstMipLevel, 1, bfDstArrayOffset, 1));

				if (istride > 1)
				{
					fx->TechniqueByName("FFTRadix8_2D_Y")->PassByIndex(0)->Bind();
					rc->Compute(numGroups, texDesc.width, 1);
					fx->TechniqueByName("FFTRadix8_2D_Y")->PassByIndex(0)->UnBind();
				}
				else
				{
					fx->TechniqueByName("FFTRadix8_2D_Y_Final")->PassByIndex(0)->Bind();
					rc->Compute(numGroups, texDesc.width, 1);
					fx->TechniqueByName("FFTRadix8_2D_Y_Final")->PassByIndex(0)->UnBind();
				}

				std::swap(bfSrcTex, bfDstTex);

				istride /= 8;
				phaseBase *= 8.0f;
			}

		}

		//Clear
		if (bfSrcTex == dst) //dst is result
		{
			bfDstTex->Release();
		}
		else //dst is not result, copy from bfSrcTex
		{
			bfSrcTex->CopyTo(dst, dstMipLevel, dstArrayOffset, 0, 0, 0, 0, 0);
			if (bfDstTex != dst)
				bfDstTex->Release();
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
		auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"FFTUtil.xml");

		fx->VariableByName("srcTex")->AsShaderResource()->SetValue(src->CreateTextureView(srcMipLevel, 1, srcArrayOffset, 1));

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ dst->CreateTextureView(dstMipLevel, 1, dstArrayOffset, 1) }, 0);

		RenderQuad(fx->TechniqueByName("FFTSetup"));

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({}, 0);
	}
}