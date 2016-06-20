#pragma once
#ifndef BOKEHDEPTHOFFIELD_H
#define BOKEHDEPTHOFFIELD_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	DECLARE_SHADER(, ComputeCoCPS, SHADER_PS, "BokehDepthOfField", "ComputeCoCPS", SM_4);
	DECLARE_SHADER(, SplitLayersPS, SHADER_PS, "BokehDepthOfField", "SplitLayersPS", SM_4);
	DECLARE_SHADER(, ComputeBokehPointsPS, SHADER_PS, "BokehDepthOfField", "ComputeBokehPointsPS", SM_4);
	DECLARE_SHADER(, RenderBokehVS, SHADER_VS, "BokehDepthOfField", "RenderBokehVS", SM_4);
	DECLARE_SHADER(, RenderBokehGS, SHADER_GS, "BokehDepthOfField", "RenderBokehGS", SM_4);
	DECLARE_SHADER(, RenderBokehPS, SHADER_PS, "BokehDepthOfField", "RenderBokehPS", SM_4);
	DECLARE_SHADER(, DOFCombinePS, SHADER_PS, "BokehDepthOfField", "DOFCombinePS", SM_4);

	DECLARE_SHADER(, BokehTileMaxXPS, SHADER_PS, "BokehDepthOfField", "TileMaxXPS", SM_4);
	DECLARE_SHADER(, BokehTileMaxYPS, SHADER_PS, "BokehDepthOfField", "TileMaxYPS", SM_4);
	DECLARE_SHADER(, PreDividingPS, SHADER_PS, "BokehDepthOfField", "PreDividingPS", SM_4);
	DECLARE_SHADER(, DOFBlurPS, SHADER_PS, "BokehDepthOfField", "DOFBlurPS", SM_4);
	DECLARE_SHADER(, DOFBlur2PS, SHADER_PS, "BokehDepthOfField", "DOFBlur2PS", SM_4);

	class Texture;
	class Camera;
	class RenderBuffer;

	class TOYGE_CORE_API BokehDepthOfField : public RenderAction
	{
	public:
		BokehDepthOfField();

		virtual void PreTAASetup(const Ptr<RenderView> & view) override;

		virtual void Render(const Ptr<RenderView> & view) override;

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

		CLASS_GET(FocalDistance, float, _focalDistance);
		CLASS_SET(FocalDistance, float, _focalDistance);

		CLASS_GET(FocalAreaLength, float, _focalAreaLength);
		CLASS_SET(FocalAreaLength, float, _focalAreaLength);

		CLASS_GET(NearAreaLength, float, _nearAreaLength);
		CLASS_SET(NearAreaLength, float, _nearAreaLength);

		CLASS_GET(FarAreaLength, float, _farAreaLength);
		CLASS_SET(FarAreaLength, float, _farAreaLength);

		CLASS_GET(MaxCoC, float, _maxCoC);
		CLASS_SET(MaxCoC, float, _maxCoC);

	private:
		Ptr<Texture> _bokehTex;
		bool _bDiskBlur;
		float _bokehIlluminanceThreshold;
		float _minBokehSize;
		float _maxBokehSize;
		float _bokehSizeScale;
		float _bokehIlluminanceScale;
		float _focalDistance;
		float _focalAreaLength;
		float _nearAreaLength;
		float _farAreaLength;
		float _maxCoC;

		PooledTextureRef ComputeCoC(
			const Ptr<RenderView> & view,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & depthTex);

		void ComputeBokehPoints(
			const Ptr<Texture> & sceneTex,
			const Ptr<Texture> & cocTex,
			PooledBufferRef & outBokehPointsBuffer,
			PooledTextureRef & outSceneTex);


		void SplitLayers(
			const Ptr<Texture> & sceneTex,
			const Ptr<Texture> & cocTex, 
			PooledTextureRef & outNearLayerTex,
			PooledTextureRef & outFarLayerTex);

		PooledTextureRef TileMax(
			const Ptr<Texture> & cocTex,
			const Ptr<Texture> & sceneDepthTex);

		PooledTextureRef PreDividing(
			const Ptr<Texture> & tileMaxTex,
			const Ptr<Texture> & cocTex,
			const Ptr<Texture> & sceneDepthTex);

		PooledTextureRef DOFBlur(
			const Ptr<Texture> & inTex, 
			const Ptr<Texture> & cocTex, 
			const Ptr<Texture> & tileMaxTex,
			const Ptr<Texture> & dividingTex,
			const Ptr<Texture> & sceneDepthTex);

		void Combine(
			const Ptr<Texture> & sceneTex,
			const Ptr<Texture> & cocTex,
			const Ptr<Texture> & nearBlurTex,
			const Ptr<Texture> & farBlurTex,
			const Ptr<RenderTargetView> & target);

		void RenderBokeh(
			const Ptr<RenderBuffer> & bokehPointsBuffer,
			const Ptr<RenderTargetView> & target);
	};
}

#endif