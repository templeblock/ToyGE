#include "ToyGE\RenderEngine\Font\SignedDistanceFieldFont.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{

	Ptr<SignedDistanceFieldFont> SignedDistanceFieldFont::Load(const Ptr<File> & file)
	{
		auto font = std::shared_ptr<SignedDistanceFieldFont>(new SignedDistanceFieldFont(file));

		auto fontName = file->Path().substr(file->ParentPath().size(), file->Path().rfind(L".") - file->ParentPath().size());

		auto basePath = Global::GetResourceManager(RESOURCE_FONT)->As<FontManager>()->BasePath();
		auto path = File::Search({ basePath, L".\\" }, fontName + L".sdff");

		if (path.size() > 0)
		{
			font->LoadSDFBin(std::make_shared<FileReader>(std::make_shared<File>(path, FILE_OPEN_READ)));
			font->_bLoadFromBin = true;
		}

		return font;
	}

	SignedDistanceFieldFont::SignedDistanceFieldFont(const Ptr<File> & file)
		: BitmapFont(file),
		_bLoadFromBin(false)
	{

	}

	void SignedDistanceFieldFont::SaveBin(const Ptr<Writer> & writer)
	{
		//for (int32_t glyphIndex = 0; glyphIndex < _numGlyphs; ++glyphIndex)
		//{
			//writer->Write(_glyphRenderInfoTable[glyphIndex]);
		//}
		writer->WriteBytes(&_glyphRenderInfoTable[0], sizeof(_glyphRenderInfoTable[0]) * _glyphRenderInfoTable.size());

		auto dumpTexDesc = _glyphRenderMapTex->Desc();
		dumpTexDesc.bindFlag = 0;
		dumpTexDesc.cpuAccess = CPU_ACCESS_READ;
		auto dumpTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(dumpTexDesc);

		writer->Write<int32_t>(dumpTexDesc.width);
		writer->Write<int32_t>(dumpTexDesc.height);
		writer->Write<int32_t>(dumpTexDesc.arraySize);
		writer->Write<int32_t>(dumpTexDesc.mipLevels);

		for (int32_t arrayIndex = 0; arrayIndex < dumpTexDesc.arraySize; ++arrayIndex)
		{
			for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
			{
				_glyphRenderMapTex->CopyTo(dumpTex, mipLevel, arrayIndex, 0, 0, 0, mipLevel, arrayIndex);
			}
		}

		for (int32_t arrayIndex = 0; arrayIndex < dumpTexDesc.arraySize; ++arrayIndex)
		{
			for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
			{
				auto dataDesc = dumpTex->Map(MAP_READ, mipLevel, arrayIndex);

				auto mipSize = dumpTex->GetMipSize(mipLevel);
				int32_t cpyPitch = std::get<0>(mipSize);

				writer->Write<size_t>(std::get<1>(mipSize) * cpyPitch);

				uint8_t * pSrc = static_cast<uint8_t*>(dataDesc.pData);
				
				for (int32_t row = 0; row < std::get<1>(mipSize); ++row)
				{
					writer->WriteBytes(pSrc, cpyPitch);
					pSrc += dataDesc.rowPitch;
				}

				dumpTex->UnMap();
			}
		}
	}

	void SignedDistanceFieldFont::Init(int32_t glyphWidth, int32_t glyphHeight)
	{
		BitmapFont::Init(glyphWidth * 4, glyphHeight * 4);

		if (_fontFace)
		{
			FT_Set_Pixel_Sizes(_fontFace, glyphWidth, glyphHeight);

			int32_t texNumSlices = 0;
			int32_t texNumCols = 0;
			int32_t texNumRows = 0;

			ComputeTexSize(glyphWidth, glyphHeight, texNumCols, texNumRows, texNumSlices);
			texNumCols /= glyphWidth;
			texNumRows /= glyphHeight;

			for (int32_t glyphIndex = 0; glyphIndex < _numGlyphs; ++glyphIndex)
			{
				if (glyphIndex == 28574)
					int a = 0;

				//Load Glyph
				auto error = FT_Load_Glyph(_fontFace, glyphIndex, 0);
				if (error)
				{
					Logger::LogLine("error> can not load glyph [font:'%s' glyph:%d]", _fontFace->family_name, glyphIndex);
					continue;
				}

				//Render Glyph
				error = FT_Render_Glyph(_fontFace->glyph, FT_RENDER_MODE_NORMAL);
				if (error)
				{
					Logger::LogLine("error> can not render glyph [font:'%s' glyph:%d]", _fontFace->family_name, glyphIndex);
					continue;
				}

				int32_t slice = glyphIndex / (texNumCols * texNumRows);
				int32_t glyphIndexInSlice = glyphIndex - slice * (texNumCols * texNumRows);
				int32_t row = glyphIndexInSlice / texNumCols;
				int32_t col = glyphIndexInSlice % texNumCols;

				//Set GlyphRenderInfo
				auto & glyphRenderInfo = _glyphRenderInfoTable[glyphIndex];
				glyphRenderInfo.renderMapUVLeft = static_cast<float>(col) / static_cast<float>(texNumCols);
				//_fontFace->glyph->bitmap_left
				glyphRenderInfo.renderMapUVRight =
					glyphRenderInfo.renderMapUVLeft
					+ (static_cast<float>(_fontFace->glyph->bitmap.width) / static_cast<float>(glyphWidth)) / static_cast<float>(texNumCols);
				glyphRenderInfo.renderMapUVTop = static_cast<float>(row) / static_cast<float>(texNumRows);
				glyphRenderInfo.renderMapUVBottom =
					glyphRenderInfo.renderMapUVTop
					+ (static_cast<float>(_fontFace->glyph->bitmap.rows) / static_cast<float>(glyphHeight)) / static_cast<float>(texNumRows);
				glyphRenderInfo.renderMapSlice = static_cast<float>(slice);

				glyphRenderInfo.glyphBearingX = static_cast<float>(_fontFace->glyph->metrics.horiBearingX) / 64.0f / static_cast<float>(glyphWidth);
				glyphRenderInfo.glyphBearingY = static_cast<float>(_fontFace->glyph->metrics.horiBearingY) / 64.0f / static_cast<float>(glyphHeight);
				glyphRenderInfo.glyphAdvanceWidth = static_cast<float>(_fontFace->glyph->metrics.horiAdvance) / 64.0f / static_cast<float>(glyphWidth);
				glyphRenderInfo.glyphWidth = static_cast<float>(_fontFace->glyph->bitmap.width) / static_cast<float>(glyphWidth);
				glyphRenderInfo.glyphHeight = static_cast<float>(_fontFace->glyph->metrics.height) / 64.0f / static_cast<float>(glyphHeight);
				//const auto & metric = GetGlyphMetrics(glyphIndex);

				//glyphRenderInfo.glyphBearingX = metric.bearingX / _fontFace->max_advance_width;// static_cast<float>(_fontFace->glyph->metrics.horiBearingX) / 64.0f / static_cast<float>(glyphWidth);
				//glyphRenderInfo.glyphBearingY = metric.bearingY / (_fontFace->ascender - _fontFace->descender);// static_cast<float>(_fontFace->glyph->metrics.horiBearingY) / 64.0f / static_cast<float>(glyphHeight);
				//glyphRenderInfo.glyphAdvanceWidth = metric.advanceWidth / _fontFace->max_advance_width;// static_cast<float>(_fontFace->glyph->metrics.horiAdvance) / 64.0f / static_cast<float>(glyphWidth);
				//glyphRenderInfo.glyphHeight = metric.height / (_fontFace->ascender - _fontFace->descender);// static_cast<float>(_fontFace->glyph->metrics.height) / 64.0f / static_cast<float>(glyphHeight);
			}


			auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"SignedDistanceFieldFont.xml");
			auto rc = Global::GetRenderEngine()->GetRenderContext();

			//DownSample
			auto sdfComputeInTexDesc = _glyphRenderMapTex->Desc();
			sdfComputeInTexDesc.bindFlag |= TEXTURE_BIND_RENDER_TARGET;
			sdfComputeInTexDesc.width /= 4;
			sdfComputeInTexDesc.height /= 4;
			auto sdfComputeInTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(sdfComputeInTexDesc);

			float4 texSize = float4(
				static_cast<float>(sdfComputeInTexDesc.width),
				static_cast<float>(sdfComputeInTexDesc.height),
				1.0f / static_cast<float>(sdfComputeInTexDesc.width),
				1.0f / static_cast<float>(sdfComputeInTexDesc.height));
			fx->VariableByName("texSize")->AsScalar()->SetValue(&texSize);

			for (int32_t sliceIndex = 0; sliceIndex < sdfComputeInTexDesc.arraySize; ++sliceIndex)
			{
				fx->VariableByName("downSampleInTex")->AsShaderResource()->SetValue(_glyphRenderMapTex->CreateTextureView(0, 1, sliceIndex, 1));

				rc->SetRenderTargets({ sdfComputeInTex->CreateTextureView(0, 1, sliceIndex, 1) }, 0);

				RenderQuad(fx->TechniqueByName("DownSample4x4"), 0, 0, sdfComputeInTexDesc.width, sdfComputeInTexDesc.height);
			}

			fx->VariableByName("downSampleInTex")->AsShaderResource()->SetValue(ResourceView());


			//Compute Edge
			auto edgeTexDesc = sdfComputeInTexDesc;
			edgeTexDesc.format = RENDER_FORMAT_R16G16_FLOAT;
			auto edgeTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(edgeTexDesc);

			uint2 uGlyphSize = uint2(static_cast<uint32_t>(glyphWidth), static_cast<uint32_t>(glyphHeight));
			fx->VariableByName("glyphSize")->AsScalar()->SetValue(&uGlyphSize);

			for (int32_t sliceIndex = 0; sliceIndex < sdfComputeInTexDesc.arraySize; ++sliceIndex)
			{
				fx->VariableByName("computeEdgeInTex")->AsShaderResource()->SetValue(sdfComputeInTex->CreateTextureView(0, 1, sliceIndex, 1));

				rc->SetRenderTargets({ edgeTex->CreateTextureView(0, 1, sliceIndex, 1) }, 0);

				RenderQuad(fx->TechniqueByName("ComputeEdgeDirection"), 0, 0, edgeTexDesc.width, edgeTexDesc.height);
			}


			//Compute Signed Distance Field Map
			TextureDesc texDesc;
			texDesc.width = texNumCols * glyphWidth;
			texDesc.height = texNumRows * glyphHeight;
			texDesc.arraySize = texNumSlices;
			texDesc.depth = 1;
			texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
			texDesc.cpuAccess = 0;
			texDesc.format = RENDER_FORMAT_R8_UNORM;
			texDesc.mipLevels = 1;
			texDesc.sampleCount = 1;
			texDesc.sampleQuality = 0;
			texDesc.type = TEXTURE_2D;

			auto sdfTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

			uint2 sdfComputeInTexSize = uint2(static_cast<uint32_t>(sdfComputeInTex->Desc().width), static_cast<uint32_t>(sdfComputeInTex->Desc().height));
			fx->VariableByName("sdfComputeInTexSize")->AsScalar()->SetValue(&sdfComputeInTexSize);

			uint2 sdfTexSize = uint2(static_cast<uint32_t>(texDesc.width), static_cast<uint32_t>(texDesc.height));
			fx->VariableByName("sdfTexSize")->AsScalar()->SetValue(&sdfTexSize);

			fx->VariableByName("sdfComputeInTex")->AsShaderResource()->SetValue(sdfComputeInTex->CreateTextureView(0, 0, 0, 0));

			fx->VariableByName("edgeTex")->AsShaderResource()->SetValue(edgeTex->CreateTextureView(0, 0, 0, 0));

			for (int32_t sliceIndex = 0; sliceIndex < texDesc.arraySize; ++sliceIndex)
			{
				uint32_t uSliceIndex = static_cast<uint32_t>(sliceIndex);
				fx->VariableByName("sliceIndex")->AsScalar()->SetValue(&uSliceIndex);

				rc->SetRenderTargets({ sdfTex->CreateTextureView(0, 1, sliceIndex, 1) }, 0);

				RenderQuad(fx->TechniqueByName("ComputeSignedDistanceField"), 0, 0, texDesc.width, texDesc.height);
			}

			sdfComputeInTex.reset();
			edgeTex.reset();

			fx->VariableByName("computeEdgeInTex")->AsShaderResource()->SetValue(ResourceView());
			fx->VariableByName("sdfComputeInTex")->AsShaderResource()->SetValue(ResourceView());
			fx->VariableByName("edgeTex")->AsShaderResource()->SetValue(ResourceView());

			auto sdfMipTexDesc = sdfTex->Desc();
			sdfMipTexDesc.mipLevels = 0;
			sdfMipTexDesc.bindFlag |= TEXTURE_BIND_GENERATE_MIPS;
			auto sdfMipTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(sdfMipTexDesc);
			for (int32_t sliceIndex = 0; sliceIndex < sdfMipTexDesc.arraySize; ++sliceIndex)
			{
				sdfTex->CopyTo(sdfMipTex, 0, sliceIndex, 0, 0, 0, 0, sliceIndex);
			}
			sdfMipTex->GenerateMips();

			_glyphRenderMapTex = sdfMipTex;
		}
	}

	void SignedDistanceFieldFont::LoadSDFBin(const Ptr<Reader> & reader)
	{
		_glyphRenderInfoTable.resize(_numGlyphs);
		reader->ReadBytes(&_glyphRenderInfoTable[0], sizeof(_glyphRenderInfoTable[0]) * _glyphRenderInfoTable.size());

		TextureDesc texDesc;
		texDesc.width = reader->Read<int32_t>();
		texDesc.height = reader->Read<int32_t>();
		texDesc.arraySize = reader->Read<int32_t>();
		texDesc.depth = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R8_UNORM;
		texDesc.mipLevels = reader->Read<int32_t>();
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_2D;

		std::vector<std::shared_ptr<uint8_t>> data;
		std::vector<RenderDataDesc> dataDesc;
		//int32_t dataIndex = 0;
		for (int32_t arrayIndex = 0; arrayIndex < texDesc.arraySize; ++arrayIndex)
		{
			int32_t w = texDesc.width;
			for (int32_t mipLevel = 0; mipLevel < texDesc.mipLevels; ++mipLevel)
			{
				auto dataSize = reader->Read<size_t>();

				data.push_back(MakeBufferedDataShared(dataSize));

				reader->ReadBytes(data.back().get(), dataSize);

				RenderDataDesc desc;
				desc.pData = data.back().get();
				desc.rowPitch = w;
				dataDesc.push_back(desc);

				w = std::max<int32_t>(1, w / 2);
			}
		}

		_glyphRenderMapTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc, dataDesc);
	}
}