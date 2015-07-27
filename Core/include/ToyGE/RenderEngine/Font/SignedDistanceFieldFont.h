#pragma once
#ifndef SIGNEDDISTANCEFIELDFONT_H
#define SIGNEDDISTANCEFIELDFONT_H

#include "ToyGE\RenderEngine\Font\BitmapFont.h"
#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{
	class SignedDistanceFieldFont : public BitmapFont
	{
	public:
		static Ptr<SignedDistanceFieldFont> Load(const Ptr<File> & file);

		void SaveBin(const Ptr<Writer> & writer);

		virtual ~SignedDistanceFieldFont() = default;

		void Init(int32_t glyphWidth, int32_t glyphHeight) override;

		bool IsBinLoaded() const
		{
			return _bLoadFromBin;
		}

	protected:
		bool _bLoadFromBin;

		SignedDistanceFieldFont(const Ptr<File> & file);

		void LoadSDFBin(const Ptr<Reader> & reader);
	};
}

#endif