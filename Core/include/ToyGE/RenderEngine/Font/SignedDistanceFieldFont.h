#pragma once
#ifndef SIGNEDDISTANCEFIELDFONT_H
#define SIGNEDDISTANCEFIELDFONT_H

#include "ToyGE\RenderEngine\Font\BitmapFont.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	DECLARE_SHADER(, SignedDistanceFieldFontComputeGradientPS, SHADER_PS, "SignedDistanceFieldFont", "ComputeGradientPS", SM_4);
	DECLARE_SHADER(, SignedDistanceFieldFontComputeSignedDistanceOffsetPS, SHADER_PS, "SignedDistanceFieldFont", "ComputeSignedDistanceOffsetPS", SM_4);
	DECLARE_SHADER(, SignedDistanceFieldFontComputeSignedDistancePS, SHADER_PS, "SignedDistanceFieldFont", "ComputeSignedDistancePS", SM_4);


	class SignedDistanceFieldFont : public BitmapFont
	{
	public:
		virtual ~SignedDistanceFieldFont() = default;

		void Init() override;

		static const String & GetCacheExtension()
		{
			static String ex = ".tsdf";
			return ex;
		}

		virtual void SaveCache() override;

		Ptr<FontRenderer> CreateRenderer() override;

	protected:
		bool _bNeedSaveCache = false;

		void InitDistanceData(int2 glyphSize, int3 glyphMapDims, std::shared_ptr<uint8_t> & data, std::vector<RenderDataDesc> & dataDescs);

		bool LoadCache();
	};
}

#endif