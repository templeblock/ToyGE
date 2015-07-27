#pragma once
#ifndef FONT_H
#define FONT_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	struct GlyphMetrics
	{
		float bearingX;
		float bearingY;
		float advanceWidth;
		float height;
	};

	class FontRenderer;

	class TOYGE_CORE_API Font
	{
	public:
		virtual ~Font() = default;

		virtual float GetAscent() const = 0;

		virtual float GetDescent() const = 0;

		virtual int32_t GetCharGlyphIndex(uint32_t charCode) const = 0;

		virtual const GlyphMetrics & GetGlyphMetrics(int32_t glyphIndex) const = 0;

		virtual Ptr<FontRenderer> CreateRenderer() = 0;
	};
}

//namespace ToyGE
//{
//	class FontImpl;
//	using FontImplPtr = std::shared_ptr < FontImpl > ;
//
//	class TOYGE_CORE_API Font : public std::enable_shared_from_this<Font>
//	{
//	public:
//		static void Init();
//
//		static Ptr<Font> Load(const Ptr<File> & file);
//
//		int32_t GetCharGlyphIndex(uint32_t charCode) const;
//
//		int32_t GetCharPage(uint32_t charCode) const;
//
//		int32_t NumPages() const;
//
//		void LoadPage(int32_t index);
//
//		bool PageReady(int32_t index) const;
//
//		GlyphPagePtr GetGlyphPage(int32_t index) const;
//
//		int32_t FontWidth() const;
//
//		int32_t FontHeight() const;
//
//		String Name() const;
//
//		Ptr<FontRenderer> CreateRenderer();
//
//		Ptr<FontRenderer> CreateRenderer(int32_t width, int32_t height);
//
//	private:
//		FontImplPtr _impl;
//	};
//}

#endif