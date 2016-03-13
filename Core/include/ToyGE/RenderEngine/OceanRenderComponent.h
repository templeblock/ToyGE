#pragma once
#ifndef OCEANRENDERCOMPONENT_H
#define OCEANRENDERCOMPONENT_H

#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	DECLARE_SHADER(, UpdateDisplacementMapPS, SHADER_PS, "OceanSimulation", "UpdateDisplacementMapPS", SM_4);
	DECLARE_SHADER(, MergeDisplacementMapPS, SHADER_PS, "OceanSimulation", "MergeDisplacementMapPS", SM_4);
	DECLARE_SHADER(, GenNormalFoldingPS, SHADER_PS, "OceanSimulation", "GenNormalFoldingPS", SM_4);

	DECLARE_SHADER(, OceanShadingVS, SHADER_VS, "OceanShading", "OceanShadingVS", SM_4);
	DECLARE_SHADER(, OceanShadingPS, SHADER_PS, "OceanShading", "OceanShadingPS", SM_4);

	class TOYGE_CORE_API OceanTileRenderComponent : public RenderComponent
	{
	public:
		OceanTileRenderComponent();

		void Init();

		virtual AABBox GetBoundsAABB() const override;

		virtual void SpecialRender(const Ptr<class RenderView> & view) override;

		CLASS_SET(Width, float, _width);
		CLASS_GET(Width, float, _width);

		CLASS_SET(Height, float, _height);
		CLASS_GET(Height, float, _height);

		CLASS_SET(TileSize, int32_t, _tileSize);
		CLASS_GET(TileSize, int32_t, _tileSize);

		CLASS_SET(DisplacementMap, Ptr<Texture>, _displacementMap);
		CLASS_SET(NormalFoldingMap, Ptr<Texture>, _normalFoldingMap);

	private:
		float _width;
		float _height;
		int32_t _tileSize;
		Ptr<Mesh> _tileMesh;

		Ptr<Texture> _displacementMap;
		Ptr<Texture> _normalFoldingMap;
	};


	class TOYGE_CORE_API OceanRenderComponent : public TransformComponent
	{
	public:
		OceanRenderComponent();

		virtual void Init() override;

		virtual void Tick(float elapsedTime) override;

		virtual void Activate() override;

	private:
		std::vector<Ptr<OceanTileRenderComponent>> _tiles;
		float _tileWidth;
		float _tileHeight;
		int32_t _tileSize;
		int2 _numTiles;

		Ptr<Texture> _h0;
		Ptr<Texture> _omega;
		Ptr<Texture> _displacementMap;
		Ptr<Texture> _normalFoldingMap;
	};

}

#endif
