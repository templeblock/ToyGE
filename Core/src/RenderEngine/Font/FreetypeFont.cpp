#include "ToyGE\RenderEngine\Font\FreetypeFont.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\Logger.h"

namespace ToyGE
{
	FT_Library FreetypeFont::_ftLibrary;

	void FreetypeFont::InitLibrary()
	{
		FT_Init_FreeType(&_ftLibrary);
	}

	void FreetypeFont::ClearLibrary()
	{
		FT_Done_FreeType(_ftLibrary);
	}

	FreetypeFont::FreetypeFont(const Ptr<File> & file)
		: _fontFace(nullptr),
		_numGlyphs(0)
	{
		if (!file)
			return;

		//Read File
		_fontData = MakeBufferedDataShared(file->Size());
		auto bytesRead = file->Read(_fontData.get(), file->Size());

		//Create Font Face
		FT_Face face;
		auto error = FT_New_Memory_Face(_ftLibrary, _fontData.get(), static_cast<FT_Long>(file->Size()), 0, &face);
		if (error == FT_Err_Unknown_File_Format)
		{
			Logger::LogLine("error> unkown font file format ['%s']", file->Path().c_str());
		}
		else if (error)
		{
			Logger::LogLine("error> can not open file ['%s']", file->Path().c_str());
		}
		else
		{
			_fontFace = face;
		}

		if (_fontFace)
		{
			_numGlyphs = _fontFace->num_glyphs;
			_ascent = static_cast<float>(_fontFace->ascender);
			_descent = static_cast<float>(_fontFace->descender);

			_glyphMetricsTable.resize(_numGlyphs);
			for (int32_t glyphIndex = 0; glyphIndex < _numGlyphs; ++glyphIndex)
			{
				//Load Glyph
				auto error = FT_Load_Glyph(_fontFace, glyphIndex, FT_LOAD_NO_SCALE);
				if (error)
				{
					Logger::LogLine("error> can not load glyph [font:'%s' glyph:%d]", _fontFace->family_name, glyphIndex);
					continue;
				}

				auto & glyphMetrics = _glyphMetricsTable[glyphIndex];
				glyphMetrics.bearingX = static_cast<float>(_fontFace->glyph->metrics.horiBearingX);
				glyphMetrics.bearingY = static_cast<float>(_fontFace->glyph->metrics.horiBearingY);
				glyphMetrics.advanceWidth = static_cast<float>(_fontFace->glyph->metrics.horiAdvance);
				glyphMetrics.height = static_cast<float>(_fontFace->glyph->metrics.height);
			}
		}

		////Set Font Pixel Size
		//error = FT_Set_Pixel_Sizes(face, g_glpyBitmapWidth, g_glpyBitmapHeight);
		//if (error)
		//{
		//	Logger::LogLine("error> can not set font size [width:%d height:%d]", g_glpyBitmapWidth, g_glpyBitmapHeight);
		//}
	}

	float FreetypeFont::GetAscent() const
	{
		/*if (_fontFace)
			return static_cast<float>(_fontFace->ascender);

		return 0.0f;*/
		return _ascent;
	}

	float FreetypeFont::GetDescent() const
	{
		/*if (_fontFace)
			return static_cast<float>(_fontFace->descender);

		return 0.0f;*/
		return _descent;
	}

	int32_t FreetypeFont::GetCharGlyphIndex(uint32_t charCode) const
	{
		return static_cast<int32_t>(FT_Get_Char_Index(_fontFace, charCode));
	}
}