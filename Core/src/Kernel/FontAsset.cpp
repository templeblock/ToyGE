#include "ToyGE\Kernel\FontAsset.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\RenderEngine\Font\FontFactory.h"

namespace ToyGE
{
	static FT_Library _ftLibrary;

	void FontAsset::InitLibrary()
	{
		FT_Init_FreeType(&_ftLibrary);
	}

	void FontAsset::ClearLibrary()
	{
		FT_Done_FreeType(_ftLibrary);
	}

	void FontAsset::Load()
	{
		if (_path.size() == 0)
		{
			ToyGE_LOG(LT_ERROR, "Try to load font with empty path!");
			return;
		}

		auto file = Global::GetPlatform()->CreatePlatformFile(GetLoadPath(), FILE_OPEN_READ);
		if (!file->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot open font file! %s", file->GetPath().c_str());
			return;
		}

		// Read font file
		_fontData = MakeBufferedDataShared(file->Size());
		auto bytesRead = file->Read(_fontData.get(), file->Size());

		// Create Font Face
		FT_Face face;
		auto error = FT_New_Memory_Face(_ftLibrary, _fontData.get(), static_cast<FT_Long>(file->Size()), 0, &face);
		if (error == FT_Err_Unknown_File_Format)
		{
			ToyGE_LOG(LT_WARNING, "Unkown font file format! %s", file->GetPath().c_str());
			return;
		}
		_fontFace = face;

		// Init font metrics
		if (_fontFace)
		{
			_numGlyphs = _fontFace->num_glyphs;
			_ascent = static_cast<float>(_fontFace->ascender);
			_descent = static_cast<float>(_fontFace->descender);

			_glyphMetricsData = std::make_shared<GlyphMetricsData>();
			_glyphMetricsData->metrics.resize(_numGlyphs);
			for (int32_t glyphIndex = 0; glyphIndex < _numGlyphs; ++glyphIndex)
			{
				//Load Glyph
				auto error = FT_Load_Glyph(_fontFace, glyphIndex, FT_LOAD_NO_SCALE);
				if (error)
				{
					ToyGE_LOG(LT_WARNING, "Cannot load glyph! font=%s glyph=%d", _fontFace->family_name, glyphIndex);
					continue;
				}

				auto & glyphMetrics = _glyphMetricsData->metrics[glyphIndex];
				glyphMetrics.bearingX = static_cast<float>(_fontFace->glyph->metrics.horiBearingX);
				glyphMetrics.bearingY = static_cast<float>(_fontFace->glyph->metrics.horiBearingY);
				glyphMetrics.advanceWidth = static_cast<float>(_fontFace->glyph->metrics.horiAdvance);
				glyphMetrics.height = static_cast<float>(_fontFace->glyph->metrics.height);
			}
		}

		_bLoaded = true;
		_bDirty = false;
		_bInit = false;
	}

	void FontAsset::Save()
	{
		if (_font)
			_font->SaveCache();

		_bDirty = false;
	}

	void FontAsset::Init()
	{
		_font = Global::GetFontFactory()->CreateNewFont();
		_font->SetAsset(shared_from_this()->Cast<FontAsset>());
		_font->Init();

		_bInit = true;
	}

	int32_t FontAsset::GetCharGlyphIndex(uint32_t charCode) const
	{
		return static_cast<int32_t>(FT_Get_Char_Index(_fontFace, charCode));
	}
}