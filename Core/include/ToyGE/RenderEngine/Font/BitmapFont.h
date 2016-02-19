#pragma once
#ifndef BITMAPFONT_H
#define BITMAPFONT_H

#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Texture;

	struct BitmapFontGlyphRenderInfo
	{
		float renderMapUVLeft;
		float renderMapUVRight;
		float renderMapUVTop;
		float renderMapUVBottom;
		float renderMapSlice;
		float glyphBearingX;
		float glyphBearingY;
		float glyphAdvanceWidth;
		float glyphWidth;
		float glyphHeight;
	};

	class TOYGE_CORE_API BitmapFont : public Font
	{
	public:
		virtual ~BitmapFont() = default;

		virtual void Init() override;

		const BitmapFontGlyphRenderInfo & GetBitmapFontGlyphRenderInfo(int32_t glyhIndex)
		{
			return _glyphRenderInfoTable[glyhIndex];
		}

		CLASS_GET(GlyphRenderMapTex, Ptr<Texture>, _glyphRenderMapTex);

		virtual Ptr<FontRenderer> CreateRenderer() override;

		CLASS_SET(GlyphSize, int2, _glyphSize);
		CLASS_GET(GlyphSize, int2, _glyphSize);

	protected:
		int2 _glyphSize = 32;
		int32_t borderSpace = 0;
		Ptr<Texture> _glyphRenderMapTex;
		std::vector<BitmapFontGlyphRenderInfo> _glyphRenderInfoTable;

		std::shared_ptr<uint8_t> _glyphMapData;
		int32_t _glyphMapCols;
		int32_t _glyphMapRows;
		int32_t _glyphMapSlices;

		void ComputeTexSize(int32_t numGlyphs, int32_t glyphWidth, int32_t glyphHeight, int32_t & outTexWidth, int32_t & outTexHeight, int32_t & outTexNumSlices);

		void InitGlyphMapDataAndRenderInfo(bool bInitMapData);

		void InitGlyphMapTex(bool bGenMips);
	};
}

#endif