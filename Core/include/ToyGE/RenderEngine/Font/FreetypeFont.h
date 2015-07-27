#pragma once
#ifndef FREETYPEFONT_H
#define FREETYPEFONT_H

#include "ToyGE\RenderEngine\Font\Font.h"
#include "boost\noncopyable.hpp"
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace ToyGE
{
	class File;

	class TOYGE_CORE_API FreetypeFont : public Font, public boost::noncopyable
	{
	public:
		static void InitLibrary();

		static void ClearLibrary();

		FreetypeFont(const Ptr<File> & file);

		virtual ~FreetypeFont() = default;

		float GetAscent() const override;

		float GetDescent() const override;

		int32_t GetCharGlyphIndex(uint32_t charCode) const override;

		const GlyphMetrics & GetGlyphMetrics(int32_t glyphIndex) const override
		{
			return _glyphMetricsTable[glyphIndex];
		}

	protected:
		FT_Face _fontFace;
		int32_t _numGlyphs;
		float _ascent;
		float _descent;
		std::vector<GlyphMetrics> _glyphMetricsTable;

	private:
		static FT_Library _ftLibrary;
		std::shared_ptr<uint8_t> _fontData;
	};
}

#endif