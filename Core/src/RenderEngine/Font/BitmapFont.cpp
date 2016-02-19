#include "ToyGE\RenderEngine\Font\BitmapFont.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Font\BitmapFontRenderer.h"
#include "ToyGE\RenderEngine\Texture.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace ToyGE
{
	void BitmapFont::Init()
	{
		if (!GetAsset())
			return;

		InitGlyphMapDataAndRenderInfo(true);
		if (!_glyphMapData)
			return;

		InitGlyphMapTex(true);
		_glyphMapData.reset();
	}

	Ptr<FontRenderer> BitmapFont::CreateRenderer()
	{
		return std::make_shared<BitmapFontRenderer>(shared_from_this()->Cast<BitmapFont>());
	}

	void BitmapFont::ComputeTexSize(int32_t numGlyphs, int32_t glyphWidth, int32_t glyphHeight, int32_t & outTexWidth, int32_t & outTexHeight, int32_t & outTexNumSlices)
	{
		int32_t texNumSlices = 0;
		int32_t texNumCols = 0;
		int32_t texNumRows = 0;

		static const int32_t texMaxUVSize = 16384;

		do
		{
			++texNumSlices;

			int32_t numGlyphsPerSlice = (numGlyphs + texNumSlices - 1) / texNumSlices;
			texNumCols = static_cast<int32_t>(std::ceil(std::sqrt(static_cast<float>(numGlyphsPerSlice))));
			texNumRows = static_cast<int32_t>(std::ceil(static_cast<float>(numGlyphsPerSlice) / static_cast<float>(texNumCols)));
		} while (texNumCols * glyphWidth > texMaxUVSize || texNumRows * glyphHeight > texMaxUVSize);

		outTexWidth = texNumCols * glyphWidth;
		outTexHeight = texNumRows * glyphHeight;
		outTexNumSlices = texNumSlices;
	}

	void BitmapFont::InitGlyphMapDataAndRenderInfo(bool bInitMapData)
	{
		auto & fontFace = GetAsset()->GetFontFace();
		if (!fontFace)
			return;

		auto glyphSize = GetGlyphSize();

		auto glyphWidth = glyphSize.x();
		auto glyphHeight = glyphSize.y();

		auto numGlyphs = GetAsset()->GetNumGlyphs();

		FT_Set_Pixel_Sizes(fontFace, glyphWidth - borderSpace * 2, glyphHeight - borderSpace * 2);

		ComputeTexSize(numGlyphs, glyphWidth, glyphHeight, _glyphMapCols, _glyphMapRows, _glyphMapSlices);
		_glyphMapCols /= glyphWidth;
		_glyphMapRows /= glyphHeight;

		_glyphRenderInfoTable.resize(numGlyphs);

		int32_t renderMapInitDataPitch = _glyphMapCols * glyphWidth;
		int32_t renderMapInitDataSlice = renderMapInitDataPitch * _glyphMapRows * glyphHeight;
		int32_t glyphMapWidth = _glyphMapCols * glyphWidth;
		int32_t glyphMapHeight = _glyphMapRows * glyphHeight;

		std::shared_ptr<uint8_t> renderMapInitData;
		if (bInitMapData)
		{
			renderMapInitData = MakeBufferedDataShared(renderMapInitDataSlice * _glyphMapSlices);
			memset(renderMapInitData.get(), 0, renderMapInitDataSlice * _glyphMapSlices);
		}

		for (int32_t glyphIndex = 0; glyphIndex < numGlyphs; ++glyphIndex)
		{
			// Load glyph
			auto error = FT_Load_Glyph(fontFace, glyphIndex, 0);
			if (error)
			{
				ToyGE_LOG(LT_WARNING, "Cannot load glyph! font=%s glyphIndex=%d", fontFace->family_name, glyphIndex);
				continue;
			}

			// Render glyph
			error = FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL);
			if (error)
			{
				ToyGE_LOG(LT_WARNING, "Cannot render glyph! font=%s glyph=%d", fontFace->family_name, glyphIndex);
				continue;
			}

			int32_t slice = glyphIndex / (_glyphMapCols * _glyphMapRows);
			int32_t glyphIndexInSlice = glyphIndex - slice * (_glyphMapCols * _glyphMapRows);
			int32_t row = glyphIndexInSlice / _glyphMapCols;
			int32_t col = glyphIndexInSlice % _glyphMapCols;

			// Init GlyphRenderInfo
			auto & glyphRenderInfo = _glyphRenderInfoTable[glyphIndex];
			glyphRenderInfo.renderMapUVLeft		= (float)col / (float)_glyphMapCols + (float)borderSpace / (float)glyphMapWidth;
			glyphRenderInfo.renderMapUVRight	= glyphRenderInfo.renderMapUVLeft + (float)fontFace->glyph->bitmap.width / (float)glyphMapWidth;
			glyphRenderInfo.renderMapUVTop		= (float)row / (float)_glyphMapRows + (float)borderSpace / (float)glyphMapHeight;
			glyphRenderInfo.renderMapUVBottom	= glyphRenderInfo.renderMapUVTop + (float)fontFace->glyph->bitmap.rows / (float)glyphMapHeight;
			glyphRenderInfo.renderMapSlice		= (float)(slice);

			glyphRenderInfo.glyphBearingX		= (float)(fontFace->glyph->metrics.horiBearingX) / 64.0f / (float)(glyphWidth - borderSpace * 2);
			glyphRenderInfo.glyphBearingY		= (float)(fontFace->glyph->metrics.horiBearingY) / 64.0f / (float)(glyphHeight - borderSpace * 2);
			glyphRenderInfo.glyphAdvanceWidth	= (float)(fontFace->glyph->metrics.horiAdvance) / 64.0f / (float)(glyphWidth - borderSpace * 2);
			glyphRenderInfo.glyphWidth			= (float)(fontFace->glyph->bitmap.width) / (float)(glyphWidth - borderSpace * 2);
			glyphRenderInfo.glyphHeight			= (float)(fontFace->glyph->metrics.height) / 64.0f / (float)(glyphHeight - borderSpace * 2);

			if (bInitMapData)
			{
				// Copy glyph bitmap data
				uint8_t * pSrcData = fontFace->glyph->bitmap.buffer;
				uint8_t * pDstData = renderMapInitData.get() + slice * renderMapInitDataSlice + (row * glyphHeight + borderSpace) * renderMapInitDataPitch + (col * glyphWidth + borderSpace);
				int32_t numCpyRows = std::min<int32_t>(glyphHeight - borderSpace * 2, fontFace->glyph->bitmap.rows);
				int32_t srcCpyPitch = std::min<int32_t>(glyphWidth - borderSpace * 2, fontFace->glyph->bitmap.pitch);
				for (int32_t row = 0; row < numCpyRows; ++row)
				{
					memcpy(pDstData, pSrcData, srcCpyPitch);
					pSrcData += fontFace->glyph->bitmap.pitch;
					pDstData += renderMapInitDataPitch;
				}
			}
		}

		_glyphMapData = renderMapInitData;
	}

	void BitmapFont::InitGlyphMapTex(bool bGenMips)
	{
		TextureDesc texDesc;
		texDesc.width = _glyphMapCols * GetGlyphSize().x();
		texDesc.height = _glyphMapRows * GetGlyphSize().y();
		texDesc.arraySize = _glyphMapSlices;
		texDesc.depth = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R8_UNORM;
		texDesc.mipLevels = 1;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		int32_t renderMapInitDataPitch = _glyphMapCols * GetGlyphSize().x();
		int32_t renderMapInitDataSlice = renderMapInitDataPitch * _glyphMapRows * GetGlyphSize().y();

		std::vector<RenderDataDesc> initDataDesc;
		for (int32_t sliceIndex = 0; sliceIndex < _glyphMapSlices; ++sliceIndex)
		{
			RenderDataDesc texInitDesc;
			texInitDesc.pData = _glyphMapData.get() + sliceIndex * renderMapInitDataSlice;
			texInitDesc.rowPitch = texDesc.width;
			initDataDesc.push_back(texInitDesc);
		}

		_glyphRenderMapTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_glyphRenderMapTex->SetDesc(texDesc);
		_glyphRenderMapTex->Init(initDataDesc);

		if (bGenMips)
		{
			_glyphRenderMapTex = _glyphRenderMapTex->CreateMips();
		}
	}
}