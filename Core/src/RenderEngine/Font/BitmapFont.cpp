#include "ToyGE\RenderEngine\Font\BitmapFont.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Font\BitmapFontRenderer.h"
#include "ToyGE\RenderEngine\Texture.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace ToyGE
{
	std::shared_ptr<BitmapFont> BitmapFont::Load(const Ptr<File> & file)
	{
		auto bitmapFont = std::shared_ptr<BitmapFont>(new BitmapFont(file));
		return bitmapFont;
	}

	BitmapFont::BitmapFont(const Ptr<File> & file)
		: FreetypeFont(file)
	{
	}

	void BitmapFont::Init(int32_t glyphWidth, int32_t glyphHeight)
	{
		if (_fontFace)
		{
			FT_Set_Pixel_Sizes(_fontFace, glyphWidth, glyphHeight);

			int32_t texNumSlices = 0;
			int32_t texNumCols = 0;
			int32_t texNumRows = 0;

			ComputeTexSize(glyphWidth, glyphHeight, texNumCols, texNumRows, texNumSlices);
			texNumCols /= glyphWidth;
			texNumRows /= glyphHeight;

			/*static const int32_t texMaxUVSize = 16384;

			do
			{
				++texNumSlices;

				int32_t numGlyphsPerSlice = (_numGlyphs + texNumSlices - 1) / texNumSlices;
				texNumCols = static_cast<int32_t>(std::ceil(std::sqrt(static_cast<float>(numGlyphsPerSlice))));
				texNumRows = static_cast<int32_t>(std::ceil(static_cast<float>(numGlyphsPerSlice) / static_cast<float>(texNumCols)));
			} while (texNumCols * glyphWidth > texMaxUVSize || texNumRows * glyphHeight > texMaxUVSize);*/

			_glyphRenderInfoTable.resize(_numGlyphs);

			int32_t renderMapInitDataPitch = texNumCols * glyphWidth;
			int32_t renderMapInitDataSlice = renderMapInitDataPitch * texNumRows * glyphHeight;
			auto renderMapInitData = MakeBufferedData(renderMapInitDataSlice * texNumSlices);
			memset(renderMapInitData.get(), 0, renderMapInitDataSlice * texNumSlices);

			for (int32_t glyphIndex = 0; glyphIndex < _numGlyphs; ++glyphIndex)
			{
				

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
				//glyphRenderInfo.glyphHeight = metric.height / (_fontFace->ascender - _fontFace->descender);

				//Copy Glyph Bitmap Data
				uint8_t * pSrcData = _fontFace->glyph->bitmap.buffer;
				uint8_t * pDstData = renderMapInitData.get() + slice * renderMapInitDataSlice + row * glyphHeight * renderMapInitDataPitch + col * glyphWidth;
				int32_t numCpyRows = std::min<int32_t>(glyphHeight, _fontFace->glyph->bitmap.rows);
				int32_t srcCpyPitch = std::min<int32_t>(glyphWidth, _fontFace->glyph->bitmap.pitch);
				for (int32_t row = 0; row < numCpyRows; ++row)
				{
					memcpy(pDstData, pSrcData, srcCpyPitch);
					pSrcData += _fontFace->glyph->bitmap.pitch;
					pDstData += renderMapInitDataPitch;
				}
			}

			//Create RenderMapTex
			TextureDesc texDesc;
			texDesc.width = texNumCols * glyphWidth;
			texDesc.height = texNumRows * glyphHeight;
			texDesc.arraySize = texNumSlices;
			texDesc.depth = 1;
			texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE;
			texDesc.cpuAccess = 0;
			texDesc.format = RENDER_FORMAT_R8_UNORM;
			texDesc.mipLevels = 1;
			texDesc.sampleCount = 1;
			texDesc.sampleQuality = 0;
			texDesc.type = TEXTURE_2D;

			std::vector<RenderDataDesc> initDataDesc;
			for (int32_t sliceIndex = 0; sliceIndex < texNumSlices; ++sliceIndex)
			{
				RenderDataDesc texInitDesc;
				texInitDesc.pData = renderMapInitData.get() + sliceIndex * renderMapInitDataSlice;
				texInitDesc.rowPitch = texDesc.width;
				initDataDesc.push_back(texInitDesc);
			}

			auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc, initDataDesc);

			texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
			texDesc.mipLevels = 0;
			_glyphRenderMapTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
			for (int32_t sliceIndex = 0; sliceIndex < texDesc.arraySize; ++sliceIndex)
				tmpTex->CopyTo(_glyphRenderMapTex, 0, sliceIndex, 0, 0, 0, 0, sliceIndex);
			_glyphRenderMapTex->GenerateMips();
		}
	}

	Ptr<FontRenderer> BitmapFont::CreateRenderer()
	{
		return std::make_shared<BitmapFontRenderer>(shared_from_this());
	}

	void BitmapFont::ComputeTexSize(int32_t glyphWidth, int32_t glyphHeight, int32_t & outTexWidth, int32_t & outTexHeight, int32_t & outTexNumSlices)
	{
		int32_t texNumSlices = 0;
		int32_t texNumCols = 0;
		int32_t texNumRows = 0;

		static const int32_t texMaxUVSize = 16384;

		do
		{
			++texNumSlices;

			int32_t numGlyphsPerSlice = (_numGlyphs + texNumSlices - 1) / texNumSlices;
			texNumCols = static_cast<int32_t>(std::ceil(std::sqrt(static_cast<float>(numGlyphsPerSlice))));
			texNumRows = static_cast<int32_t>(std::ceil(static_cast<float>(numGlyphsPerSlice) / static_cast<float>(texNumCols)));
		} while (texNumCols * glyphWidth > texMaxUVSize || texNumRows * glyphHeight > texMaxUVSize);

		outTexWidth = texNumCols * glyphWidth;
		outTexHeight = texNumRows * glyphHeight;
		outTexNumSlices = texNumSlices;
	}
}