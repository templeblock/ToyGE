#pragma once
#ifndef BOKEHDEPTHOFFIELD_H
#define BOKEHDEPTHOFFIELD_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class Texture;
	class Camera;
	class RenderBuffer;

	class TOYGE_CORE_API BokehDepthOfField : public RenderAction
	{
	public:
		BokehDepthOfField();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_GET(BokehIlluminanceThrehold, float, _bokehIlluminanceThreshold);
		CLASS_SET(BokehIlluminanceThrehold, float, _bokehIlluminanceThreshold);

		CLASS_GET(MinBokehSize, float, _minBokehSize);
		CLASS_SET(MinBokehSize, float, _minBokehSize);

		CLASS_GET(MaxBokehSize, float, _maxBokehSize);
		CLASS_SET(MaxBokehSize, float, _maxBokehSize);

		CLASS_GET(BokeSizeScale, float, _bokehSizeScale);
		CLASS_SET(BokeSizeScale, float, _bokehSizeScale);

		CLASS_GET(BokehIlluminanceScale, float, _bokehIlluminanceScale);
		CLASS_SET(BokehIlluminanceScale, float, _bokehIlluminanceScale);

	private:
		Ptr<RenderEffect> _fx;
		Ptr<Texture> _bokehTex;
		bool _bDiskBlur;
		float _bokehIlluminanceThreshold;
		float _minBokehSize;
		float _maxBokehSize;
		float _bokehSizeScale;
		float _bokehIlluminanceScale;

		Ptr<Texture> ComputeCoC(const Ptr<Texture> & linearDepth, const Ptr<Texture> & rawDepth, const Ptr<Camera> & camera);

		std::pair<Ptr<Texture>, Ptr<Texture>> SplitLayers(const Ptr<Texture> & cocTex, const Ptr<Texture> & sceneTex);

		Ptr<Texture> DownSample(const Ptr<Texture> & inputTex);

		Ptr<Texture> DOFDiskBlur(const Ptr<Texture> & inputTex);

		std::pair<Ptr<RenderBuffer>, Ptr<Texture>> ComputeBokehPoints(const Ptr<Texture> & inputTex);

		Ptr<Texture> RenderBokeh(const std::initializer_list<Ptr<Texture>> & targets, const Ptr<RenderBuffer> & bokehPointsBuffer);

		void Recombine(
			const std::vector<Ptr<Texture>> & nearBlurTex,
			const std::vector<Ptr<Texture>> & farBlurTex,
			const Ptr<Texture> & bokehTex,
			const ResourceView & target);
	};
}

#endif