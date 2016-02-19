#pragma once
#ifndef FONTASSET_H
#define FONTASSET_H

#include "ToyGE\Kernel\Asset.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace ToyGE
{
	struct GlyphMetrics
	{
		float bearingX;
		float bearingY;
		float advanceWidth;
		float height;
	};

	struct GlyphMetricsData
	{
		std::vector<GlyphMetrics> metrics;
	};

	class TOYGE_CORE_API FontAsset : public Asset
	{
	public:
		virtual ~FontAsset() = default;

		// Load assets
		virtual void Load() override;

		// Save assets
		virtual void Save() override;

		virtual void Init() override;

		virtual float GetAscent() const
		{
			return _ascent;
		}

		virtual float GetDescent() const
		{
			return _descent;
		}

		virtual int32_t GetNumGlyphs() const
		{
			return _numGlyphs;
		}

		virtual int32_t GetCharGlyphIndex(uint32_t charCode) const;

		virtual GlyphMetrics GetGlyphMetrics(int32_t glyphIndex) const
		{
			return _glyphMetricsData ? _glyphMetricsData->metrics[glyphIndex] : GlyphMetrics{0};
		}

		const Ptr<GlyphMetricsData> & GetGlyphMetricsData(int32_t glyphIndex) const
		{
			return _glyphMetricsData;
		}

		CLASS_GET(FontFace, FT_Face, _fontFace);

		CLASS_GET(Font, Ptr<class Font>, _font);

		static Ptr<FontAsset> New()
		{
			return std::shared_ptr<FontAsset>(new FontAsset);
		}

		static void InitLibrary();

		static void ClearLibrary();

	protected:
		std::shared_ptr<uint8_t> _fontData;
		FT_Face _fontFace;

		int32_t _numGlyphs;
		float _ascent;
		float _descent;
		Ptr<GlyphMetricsData> _glyphMetricsData;

		Ptr<class Font> _font;
	};
}

#endif