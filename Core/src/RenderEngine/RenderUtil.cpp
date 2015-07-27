#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderFormat.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderTechnique.h"
#include "ToyGE\RenderEngine\RenderPass.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffectVariable.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\Math\Math.h"

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

	Ptr<Texture> HeightToNormal(const Ptr<Texture> & heightTex)
	{
		auto preInput = Global::GetRenderEngine()->GetRenderContext()->GetRenderInput();
		auto quadInput = CommonInput::QuadInput();
		Global::GetRenderEngine()->GetRenderContext()->SetRenderInput(quadInput);

		TextureDesc texDesc = heightTex->Desc();
		texDesc.format = RENDER_FORMAT_R8G8B8A8_UNORM;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
		auto normalTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		auto preRTs = Global::GetRenderEngine()->GetRenderContext()->GetRenderTargets();
		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ normalTex->CreateTextureView() }, 0);

		auto preVp = Global::GetRenderEngine()->GetRenderContext()->GetViewport();
		RenderViewport vp;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		Global::GetRenderEngine()->GetRenderContext()->SetViewport(vp);

		auto preDepthStencil = Global::GetRenderEngine()->GetRenderContext()->GetDepthStencil();
		Global::GetRenderEngine()->GetRenderContext()->SetDepthStencil(ResourceView());

		auto effect = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"HeightToNormal.xml");
		effect->VariableByName("height_tex")->AsShaderResource()->SetValue(heightTex->CreateTextureView());
		int2 imageSize = int2(texDesc.width, texDesc.height);
		effect->VariableByName("imageSize")->AsScalar()->SetValue(&imageSize, sizeof(imageSize));
		effect->TechniqueByIndex(0)->PassByIndex(0)->Bind();
		Global::GetRenderEngine()->GetRenderContext()->DrawIndexed();
		effect->TechniqueByIndex(0)->PassByIndex(0)->UnBind();

		Global::GetRenderEngine()->GetRenderContext()->SetViewport(preVp);
		Global::GetRenderEngine()->GetRenderContext()->SetDepthStencil(preDepthStencil);
		Global::GetRenderEngine()->GetRenderContext()->SetRenderInput(preInput);
		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets(preRTs, 0);

		normalTex->GenerateMips();

		return normalTex;
	}

	Ptr<Texture> SpecularToRoughness(const Ptr<Texture> & shininessTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();
		RenderContextStateSave stateSave;
		rc->SaveState(RENDER_CONTEXT_STATE_INPUT | RENDER_CONTEXT_STATE_RENDERTARGETS | RENDER_CONTEXT_STATE_VIEWPORT, stateSave);

		auto texDesc = shininessTex->Desc();
		texDesc.format = RENDER_FORMAT_R8_UNORM;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
		auto roughnessTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

		RenderViewport vp;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"SpecularToRoughness.xml");
		fx->VariableByName("specularTex")->AsShaderResource()->SetValue(shininessTex->CreateTextureView());
		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetRenderTargets({ roughnessTex->CreateTextureView() }, 0);
		rc->SetDepthStencil(ResourceView());
		fx->TechniqueByIndex(0)->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		fx->TechniqueByIndex(0)->PassByIndex(0)->UnBind();

		roughnessTex->GenerateMips();

		rc->RestoreState(stateSave);

		return roughnessTex;
	}

	void Transform(
		const ResourceView & src,
		const ResourceView & dst,
		ColorWriteMask colorMask,
		const int4 & dstRect)
	{
		auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"Transform.xml");

		if (colorMask & COLOR_WRITE_R)
			fx->AddExtraMacro("COLOR_WRITE_R", "");
		else
			fx->RemoveExtraMacro("COLOR_WRITE_R");

		if (colorMask & COLOR_WRITE_G)
			fx->AddExtraMacro("COLOR_WRITE_G", "");
		else
			fx->RemoveExtraMacro("COLOR_WRITE_G");

		if (colorMask & COLOR_WRITE_B)
			fx->AddExtraMacro("COLOR_WRITE_B", "");
		else
			fx->RemoveExtraMacro("COLOR_WRITE_B");

		if (colorMask & COLOR_WRITE_A)
			fx->AddExtraMacro("COLOR_WRITE_A", "");
		else
			fx->RemoveExtraMacro("COLOR_WRITE_A");

		fx->UpdateData();

		/*switch (colorMask)
		{
		case COLOR_WRITE_R:
			fx->SetExtraMacros({ { "COLOR_WRITE_R", "" } });
			break;
		case COLOR_WRITE_G:
			fx->SetExtraMacros({ { "COLOR_WRITE_G", "" } });
			break;
		case COLOR_WRITE_B:
			fx->SetExtraMacros({ { "COLOR_WRITE_B", "" } });
			break;
		case COLOR_WRITE_A:
			fx->SetExtraMacros({ { "COLOR_WRITE_A", "" } });
			break;
		default:
			fx->SetExtraMacros({ { "", "" } });
			break;
		}*/

		fx->VariableByName("srcTex")->AsShaderResource()->SetValue(src);

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ dst }, 0);

		auto dstTex = std::static_pointer_cast<Texture>(dst.resource);

		RenderQuad(fx->TechniqueByName("Transform"), 
			dstRect.x < 0 ? 0 : dstRect.x,
			dstRect.y < 0 ? 0 : dstRect.y,
			dstRect.z < 0 ? dstTex->Desc().width : dstRect.z,
			dstRect.w < 0 ? dstTex->Desc().height : dstRect.w);
	}

	Ptr<Texture> SAT(const Ptr<Texture> & tex)
	{
		auto satFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"SAT.xml");

		TextureDesc texDesc = tex->Desc();
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;

		auto tex0 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto tex1 = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		for (int32_t arrayIndex = 0; arrayIndex < texDesc.arraySize; ++arrayIndex)
		{
			tex->CopyTo(tex0, 0, arrayIndex, 0, 0, 0, 0, arrayIndex);
		}
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto quadInput = CommonInput::QuadInput();
		auto preInput = rc->GetRenderInput();
		rc->SetRenderInput(quadInput);

		auto preRts = rc->GetRenderTargets();
		auto preDS = rc->GetDepthStencil();
		rc->SetDepthStencil(ResourceView());

		auto preViewport = rc->GetViewport();
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		int offset = 1;
		int horzCnt = static_cast<int>(log2(texDesc.width));
		for (int horzIndex = 0; horzIndex < horzCnt; ++horzIndex)
		{
			for (int32_t arrayIndex = 0; arrayIndex < texDesc.arraySize; ++arrayIndex)
			{
				satFX->VariableByName("offset")->AsScalar()->SetValue(&offset, sizeof(offset));
				satFX->VariableByName("inTex")->AsShaderResource()->SetValue(tex0->CreateTextureView(0, 1, arrayIndex, 1));
				Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ tex1->CreateTextureView(0, 1, arrayIndex, 1) }, 0);

				satFX->TechniqueByName("SAT_Horz")->PassByIndex(0)->Bind();
				Global::GetRenderEngine()->GetRenderContext()->DrawIndexed();
				satFX->TechniqueByName("SAT_Horz")->PassByIndex(0)->UnBind();
			}

			tex0.swap(tex1);

			offset = offset << 1;
		}

		offset = 1;
		int vertCnt = static_cast<int>(log2(texDesc.height));
		for (int vertIndex = 0; vertIndex < vertCnt; ++vertIndex)
		{
			for (int32_t arrayIndex = 0; arrayIndex < texDesc.arraySize; ++arrayIndex)
			{
				satFX->VariableByName("offset")->AsScalar()->SetValue(&offset, sizeof(offset));
				satFX->VariableByName("inTex")->AsShaderResource()->SetValue(tex0->CreateTextureView(0, 1, arrayIndex, 1));
				Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ tex1->CreateTextureView(0, 1, arrayIndex, 1) }, 0);

				satFX->TechniqueByName("SAT_Vert")->PassByIndex(0)->Bind();
				Global::GetRenderEngine()->GetRenderContext()->DrawIndexed();
				satFX->TechniqueByName("SAT_Vert")->PassByIndex(0)->UnBind();
			}

			tex0.swap(tex1);

			offset = offset << 1;
		}

		rc->SetRenderInput(preInput);
		rc->SetRenderTargets(preRts, 0);
		rc->SetDepthStencil(preDS);
		rc->SetViewport(preViewport);

		tex1->Release();

		return tex0;
	}

	Ptr<Texture> DownSample(const ResourceView & texView, float2 scale)
	{
		auto tex = std::static_pointer_cast<Texture>(texView.resource);
		auto texDesc = tex->Desc();
		texDesc.bindFlag |= TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.width = static_cast<int>(static_cast<float>(texDesc.width) * scale.x);
		texDesc.height = static_cast<int>(static_cast<float>(texDesc.height) * scale.y);

		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		RenderContextStateSave stateSave;
		rc->SaveState(
			RENDER_CONTEXT_STATE_INPUT
			| RENDER_CONTEXT_STATE_VIEWPORT
			| RENDER_CONTEXT_STATE_RENDERTARGETS
			| RENDER_CONTEXT_STATE_DEPTHSTENCIL, stateSave);

		RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		auto downSampleFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"DownSample.xml");
		downSampleFX->VariableByName("inTex")->AsShaderResource()->SetValue(texView);
		rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
		downSampleFX->TechniqueByIndex(0)->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		downSampleFX->TechniqueByIndex(0)->PassByIndex(0)->UnBind();

		rc->RestoreState(stateSave);

		return resultTex;
	}

	void RenderQuad(
		const Ptr<RenderTechnique> & tech,
		int32_t topLeftX,
		int32_t topLeftY,
		int32_t width,
		int32_t height,
		const ResourceView & depthStencil)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		RenderContextStateSave stateSave;
		rc->SaveState(
			RENDER_CONTEXT_STATE_INPUT
			| RENDER_CONTEXT_STATE_VIEWPORT
			| RENDER_CONTEXT_STATE_RENDERTARGETS
			| RENDER_CONTEXT_STATE_DEPTHSTENCIL, stateSave);

		auto & rts = Global::GetRenderEngine()->GetRenderContext()->GetRenderTargets();

		if (rts.size() > 0)
		{
			auto targetTex = std::static_pointer_cast<Texture>(rts[0].resource);
			if (width == 0)
				width = targetTex->Desc().width;
			if (height == 0)
				height = targetTex->Desc().height;
		}

		RenderViewport vp;
		vp.topLeftX = static_cast<float>(topLeftX);
		vp.topLeftY = static_cast<float>(topLeftY);
		vp.width = static_cast<float>(width);
		vp.height = static_cast<float>(height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(depthStencil);

		for (int32_t i = 0; i < tech->NumPasses(); ++i)
		{
			tech->PassByIndex(i)->Bind();
			rc->DrawIndexed();
			tech->PassByIndex(i)->UnBind();
		}

		rc->RestoreState(stateSave);
	}

	void TextureFilter(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t numSamples,
		const std::vector<float2> & uvOffsets,
		const std::vector<float> & weights)
	{
		MacroDesc macro;
		macro.name = "NUM_SAMPLES";
		macro.value = std::to_string(numSamples);

		if (numSamples == 5)
		{
			int a = 0;
		}

		auto filterFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"Filter.xml");
		filterFX->SetExtraMacros({ macro });

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		RenderContextStateSave stateSave;
		rc->SaveState(
			RENDER_CONTEXT_STATE_INPUT
			| RENDER_CONTEXT_STATE_VIEWPORT
			| RENDER_CONTEXT_STATE_RENDERTARGETS
			| RENDER_CONTEXT_STATE_DEPTHSTENCIL, stateSave);

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		auto & mipSize = dst->GetMipSize(dstMipLevel);
		vp.width = static_cast<float>(std::get<0>(mipSize));
		vp.height = static_cast<float>(std::get<1>(mipSize));
		rc->SetViewport(vp);

		filterFX->VariableByName("samplesOffsets")->AsScalar()->SetValue(&uvOffsets[0], sizeof(float2) * static_cast<size_t>(numSamples));
		filterFX->VariableByName("samplesWeights")->AsScalar()->SetValue(&weights[0], sizeof(float) * static_cast<size_t>(numSamples));

		filterFX->VariableByName("filterTex")->AsShaderResource()->SetValue(src->CreateTextureView(srcMipLevel, 1, srcArrayOffset, 1));

		rc->SetDepthStencil(ResourceView());
		rc->SetRenderTargets({ dst->CreateTextureView(dstMipLevel, 1, dstArrayOffset, 1) }, 0);
		rc->SetRenderInput(CommonInput::QuadInput());

		filterFX->TechniqueByName("Filter")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		filterFX->TechniqueByName("Filter")->PassByIndex(0)->UnBind();

		rc->RestoreState(stateSave);
	}

	void BilateralUpSampling(
		const ResourceView & src,
		const ResourceView & lowResDepthTex,
		const ResourceView & highResDepthTex,
		const ResourceView & dst,
		float depthDiffScale)
	{
		auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"BilateralFilter.xml");

		fx->VariableByName("depthDiffScale")->AsScalar()->SetValue(&depthDiffScale);

		fx->VariableByName("upSamplingInTex")->AsShaderResource()->SetValue(src);
		fx->VariableByName("lowResDepthTex")->AsShaderResource()->SetValue(lowResDepthTex);
		fx->VariableByName("highResDepthTex")->AsShaderResource()->SetValue(highResDepthTex);

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ dst }, 0);

		int32_t w = std::static_pointer_cast<Texture>(dst.resource)->Desc().width;
		int32_t h = std::static_pointer_cast<Texture>(dst.resource)->Desc().height;

		RenderQuad(fx->TechniqueByName("BilateralUpSampling"), 0, 0, w, h);
	}

	bool ResourceView::operator == (const ResourceView & view) const
	{
		if (resource != view.resource)
			return false;
		if (formatHint != view.formatHint)
			return false;
		if (!resource)
			return true;
		if (RENDER_RESOURCE_BUFFER == resource->ResourceType())
		{
			return subDesc.bufferDesc.firstElement == view.subDesc.bufferDesc.firstElement
				&& subDesc.bufferDesc.numElements == view.subDesc.bufferDesc.numElements;
		}
		else if (RENDER_RESOURCE_TEXTURE == resource->ResourceType())
		{
			if (subDesc.textureDesc.bAsCube == true && view.subDesc.textureDesc.bAsCube == true)
			{
				return subDesc.textureDesc.firstMipLevel == view.subDesc.textureDesc.firstMipLevel
					&& subDesc.textureDesc.mipLevels == view.subDesc.textureDesc.mipLevels
					&& subDesc.textureDesc.firstFaceOffset == view.subDesc.textureDesc.firstFaceOffset
					&& subDesc.textureDesc.numCubes == view.subDesc.textureDesc.numCubes;
			}
			else if (subDesc.textureDesc.bAsCube == false && view.subDesc.textureDesc.bAsCube == false)
			{
				return subDesc.textureDesc.firstMipLevel == view.subDesc.textureDesc.firstMipLevel
					&& subDesc.textureDesc.mipLevels == view.subDesc.textureDesc.mipLevels
					&& subDesc.textureDesc.firstArray == view.subDesc.textureDesc.firstArray
					&& subDesc.textureDesc.arraySize == view.subDesc.textureDesc.arraySize;
			}
			else
			{
				return false;
			}
		}

		return false;
	}
}
