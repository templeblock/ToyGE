#pragma once
#ifndef BITMAPFONT_H
#define BITMAPFONT_H

#include "ToyGE\RenderEngine\Font\FreetypeFont.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class BitmapFont;
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

	class TOYGE_CORE_API BitmapFont : public FreetypeFont, public std::enable_shared_from_this<BitmapFont>
	{
	public:
		static Ptr<BitmapFont> Load(const Ptr<File> & file);

		virtual void Init(int32_t glyphWidth, int32_t glyphHeight);

		virtual ~BitmapFont() = default;

		float2 GetBitmapGlyphSize();

		const BitmapFontGlyphRenderInfo & GetBitmapFontGlyphRenderInfo(int32_t glyhIndex)
		{
			return _glyphRenderInfoTable[glyhIndex];
		}

		CLASS_GET(RenderMapTex, Ptr<Texture>, _glyphRenderMapTex);

		Ptr<FontRenderer> CreateRenderer() override;

	protected:
		Ptr<Texture> _glyphRenderMapTex;
		std::vector<BitmapFontGlyphRenderInfo> _glyphRenderInfoTable;

		BitmapFont(const Ptr<File> & file);

		void ComputeTexSize(int32_t glyphWidth, int32_t glyphHeight, int32_t & outTexWidth, int32_t & outTexHeight, int32_t & outTexNumSlices);
	};
}

#endif