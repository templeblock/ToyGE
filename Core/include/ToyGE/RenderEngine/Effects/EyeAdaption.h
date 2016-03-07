#pragma once
#ifndef EYEADAPTION_H
#define EYEADAPTION_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	DECLARE_SHADER(, BuildHistogramCS, SHADER_CS, "HistogramBuild", "BuildHistogramCS", SM_5);
	DECLARE_SHADER(, MergeHistogramPS, SHADER_PS, "HistogramMerge", "MergeHistogramPS", SM_4);
	DECLARE_SHADER(, EyeAdaptionPS, SHADER_PS, "EyeAdaption", "EyeAdaptionPS", SM_4);

	class TOYGE_CORE_API EyeAdaption : public RenderAction
	{
	public:

		virtual void Render(const Ptr<RenderView> & view) override;

	private:
		//PooledTextureRef _prevAdaptedExposureScaleRef;
		//PooledTextureRef _adaptedExposureScaleRef;

		PooledTextureRef BuildHistogram(const Ptr<Texture> & scene);

		void ComputeEyeAdaption(
			const Ptr<Texture> & histogram,
			const Ptr<Texture> & prevAdaptedExposureScale,
			const Ptr<Texture> & adaptedExposureScale);
	};
}

#endif
