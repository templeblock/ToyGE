#pragma once
#ifndef MESH_H
#define MESH_H

#include "ToyGE\Kernel\MeshAsset.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Material;
	class Scene;
	class RenderMeshComponent;

	class TOYGE_CORE_API Mesh : public std::enable_shared_from_this<Mesh>
	{
		friend class MeshAsset;
	public:
		/** Init from data */
		void Init();

		void SetData(const std::vector<Ptr<MeshElement>> & data) 
		{ 
			_data = data; 
			_bDirty = true;
		}
		const std::vector<Ptr<MeshElement>> & GetData() const
		{
			return _data;
		}

		const Ptr<class MeshRenderData> & GetRenderData() const
		{
			return _renderData;
		}

		void SetAsset(const Ptr<MeshAsset> & asset)
		{
			if (_asset.lock() != asset)
			{
				_asset = asset;
				_bDirty = true;
			}
		}
		Ptr<MeshAsset> GetAsset() const
		{
			return _asset.lock();
		}

		Ptr<RenderMeshComponent> AddInstanceToScene(
			const Ptr<Scene> & scene,
			const XMFLOAT3 & pos = XMFLOAT3(0.0f, 0.0f, 0.0f),
			const XMFLOAT3 & scale = XMFLOAT3(1.0f, 1.0f, 1.0f),
			const XMFLOAT4 & orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

		void SetDirty(bool bDirty)
		{
			_bDirty = bDirty;
		}
		bool IsDirty() const;

		void UpdateFromRenderData();

	private:
		std::vector<Ptr<MeshElement>> _data;
		Ptr<class MeshRenderData> _renderData;

		std::weak_ptr<MeshAsset> _asset;
		bool _bDirty = false;

		void InitDepthRenderData();
	};

	class TOYGE_CORE_API MeshElementRenderData
	{
		friend class Mesh;
	public:
		Ptr<MeshElement> GetElementData() const
		{
			return _meshElementData.lock();
		}

		CLASS_GET(VertexBuffer, std::vector< Ptr<class VertexBuffer> >, _vertexBuffers);

		CLASS_GET(DepthVertexBuffer, std::vector< Ptr<class VertexBuffer> >, _depthVertexBuffer);

		CLASS_GET(IndexBuffer, Ptr<class RenderBuffer>, _indexBuffer);

		CLASS_GET(WeightBuffer, Ptr<class VertexBuffer>, _weightBuffer);

		void SetMaterial(const Ptr<class Material> & material)
		{
			_material = material;
			_bDirty = true;
		}
		CLASS_GET(Material, Ptr<class Material>, _material);

		CLASS_SET(Dirty, bool, _bDirty);
		bool IsDirty() const;

		void BindMacros(std::map<String, String> & outMacros);

		void BindShaderParams(const Ptr<class Shader> & shader);

		void Draw();

		void BindDepthMacros(bool bWithOpacityMask, std::map<String, String> & outMacros);

		void BindDepthShaderParams(const Ptr<class Shader> & shader, bool bWithOpacityMask);

		void DrawDepth(bool bWithOpacityMask);

		void InitWeightBuffer();

	private:
		std::weak_ptr<MeshElement> _meshElementData;
		std::vector< Ptr<class VertexBuffer> > _vertexBuffers;
		std::vector< Ptr<class VertexBuffer> > _depthVertexBuffer;
		Ptr<class VertexBuffer> _weightBuffer;
		Ptr<class RenderBuffer> _indexBuffer;
		Ptr<class Material> _material;
		bool _bDirty = false;
	};

	class TOYGE_CORE_API MeshRenderData
	{
		friend class Mesh;
	public:
		CLASS_GET(MeshElements, std::vector<Ptr<MeshElementRenderData>>, _meshElements);

		bool IsDirty() const
		{
			for (auto & element : _meshElements)
				if (element->IsDirty())
					return true;
			return false;
		}

	private:
		std::vector<Ptr<MeshElementRenderData>> _meshElements;
	};

	class TOYGE_CORE_API CommonMesh
	{
	public:
		static Ptr<Mesh> CreatePlane(float width, float height, int32_t uSplits, int32_t vSplits);

		static Ptr<Mesh> CreateSphere(float radius, int32_t numSplits);

		static Ptr<Mesh> CreateCone(float height, float angle, int32_t numSplits);

		static Ptr<Mesh> CreateCube(float width, float height, float depth);
	};
}

#endif