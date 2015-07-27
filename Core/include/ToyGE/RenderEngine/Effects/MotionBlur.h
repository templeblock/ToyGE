#pragma once
#ifndef MORIONBLUR_H
#define MORIONBLUR_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class TOYGE_CORE_API MotionBlur : public RenderAction
	{
	public:
		MotionBlur();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_GET(ExposureTime, float, _exposureTime);
		CLASS_SET(ExposureTime, float, _exposureTime);

	private:
		Ptr<RenderEffect> _fx;
		float _exposureTime;
		int32_t _maxVelocityLength;
		int32_t _mbNumSamples;

		Ptr<Texture> InitVelocityMap(const Ptr<Texture> & rawVelocityTex);

		Ptr<Texture> TileMax(const Ptr<Texture> & velocityTex);

		Ptr<Texture> NeighborMax(const Ptr<Texture> & tileMaxTex);

		void Blur(
			const Ptr<Texture> & sceneTex,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & velocityTex,
			const Ptr<Texture> & neighborMaxTex,
			const ResourceView & target);
	};
}

#endif