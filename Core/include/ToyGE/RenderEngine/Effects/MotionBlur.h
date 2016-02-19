#pragma once
#ifndef MORIONBLUR_H
#define MORIONBLUR_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	DECLARE_SHADER(, InitVelocityMapPS, SHADER_PS, "MotionBlur", "InitVelocityMapPS", SM_4);
	DECLARE_SHADER(, TileMaxXPS, SHADER_PS, "MotionBlur", "TileMaxXPS", SM_4);
	DECLARE_SHADER(, TileMaxYPS, SHADER_PS, "MotionBlur", "TileMaxYPS", SM_4);
	DECLARE_SHADER(, NeighborMaxPS, SHADER_PS, "MotionBlur", "NeighborMaxPS", SM_4);
	DECLARE_SHADER(, MotionBlurPS, SHADER_PS, "MotionBlur", "MotionBlurPS", SM_4);

	class TOYGE_CORE_API MotionBlur : public RenderAction
	{
	public:
		MotionBlur();

		void Render(const Ptr<RenderView> & view) override;

		CLASS_GET(ExposureTime, float, _exposureTime);
		CLASS_SET(ExposureTime, float, _exposureTime);

	private:
		float _exposureTime;
		int32_t _maxVelocityLength;
		int32_t _mbNumSamples;

		PooledTextureRef InitVelocityMap(const Ptr<Texture> & rawVelocityTex);

		PooledTextureRef TileMax(const Ptr<Texture> & velocityTex);

		PooledTextureRef NeighborMax(const Ptr<Texture> & tileMaxTex);

		void Blur(
			const Ptr<Texture> & sceneTex,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & velocityTex,
			const Ptr<Texture> & neighborMaxTex,
			const Ptr<RenderTargetView> & target);
	};
}

#endif