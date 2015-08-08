#pragma once
#ifndef MESH_H
#define MESH_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{
	class RenderInput;
	class Material;
	class Scene;
	class RenderComponent;

	struct StandardVertex
	{
		using pos_type = float3;
		using texCoord_type = float3;
		using normal_type = float3;
		using tangent_type = float3;

		float3 pos;
		std::array<float3, 8> texCoord;
		float3 normal;
		float3 tangent;
	};

	class VertexDataDesc
	{
	public:
		std::shared_ptr<uint8_t> pData;
		int32_t numVertices;
		int32_t vertexByteSize;
		std::vector<VertexElementDesc> elementsDesc;

		VertexDataDesc()
			: pData(nullptr),
			numVertices(0),
			vertexByteSize(0)
		{
		}

		int32_t FindElementDesc(const String & name) const
		{
			int32_t index = 0;
			for (auto & elem : elementsDesc)
			{
				if (elem.name == name)
					return index;
				++index;
			}
			return -1;
		}

		template <typename T>
		T & GetElement(int32_t vertexIndex, const VertexElementDesc & elemDesc)
		{
			return *(reinterpret_cast<T*>(pData.get() + vertexIndex * vertexByteSize + elemDesc.bytesOffset));
		}
	};

	class VertexDataBuildHelp
	{
	public:
		VertexDataDesc vertexDataDesc;

		VertexDataBuildHelp();

		void AddElementDesc(const String & name, int32_t index, RenderFormat format, int32_t instanceRate);

		void SetNumVertices(int32_t numVertices);

		void Start();

		template <typename T>
		void Add(const T & value)
		{
			memcpy(_pBufferData, &value, sizeof(T));
			_pBufferData += sizeof(T);
		}

		void Finish();

	private:
		std::shared_ptr<uint8_t> _buffer;
		uint8_t * _pBufferData;
		int32_t _bytesOffset;
	};

	class StandardVertexElementName
	{
	public:
		static String Position();

		static String TextureCoord();

		static String Normal();

		static String Tangent();
	};

	class TOYGE_CORE_API Mesh : public std::enable_shared_from_this<Mesh>
	{
		friend class RenderMesh;
	public:
		void SaveBin(const Ptr<Writer> & writer, const std::map<Ptr<Material>, int> & materials);

		static Ptr<Mesh> LoadBin(const Ptr<Reader> & reader, const std::vector<Ptr<Material>> & materials);

		void SetName(const WString & name)
		{
			_name = name;
		}

		const WString & Name() const
		{
			return _name;
		}

		int32_t AddVertexData(const VertexDataDesc & vertexData)
		{
			_vertexDatas.push_back(vertexData);
			return static_cast<int32_t>(_vertexDatas.size() - 1);
		}

		int32_t NumVertexSlots() const
		{
			return static_cast<int32_t>(_vertexDatas.size());
		}

		VertexDataDesc & GetVertexData(int32_t slotIndex)
		{
			return _vertexDatas[slotIndex];
		}

		void AddIndex(uint32_t index)
		{
			_indices.push_back(index);
		}

		int32_t NumIndices() const
		{
			return static_cast<uint32_t>(_indices.size());
		}

		uint32_t GetIndex(int32_t i)
		{
			return _indices[i];
		}

		const Ptr<RenderMesh> & InitRenderData();

		const Ptr<RenderMesh> & GetRender() const
		{
			return _renderData;
		}

		const Ptr<RenderMesh> & AcquireRender()
		{
			if (!_renderData)
				return InitRenderData();
			else
				return GetRender();
		}

		Ptr<RenderComponent> AddInstanceToScene(
			const Ptr<Scene> & scene,
			const XMFLOAT3 & pos = XMFLOAT3(0.0f, 0.0f, 0.0f),
			const XMFLOAT3 & scale = XMFLOAT3(1.0f, 1.0f, 1.0f),
			const XMFLOAT4 & orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

		Ptr<Mesh> Copy() const;

	private:
		WString _name;
		std::vector<VertexDataDesc> _vertexDatas;
		std::vector<uint32_t> _indices;
		Ptr<RenderMesh> _renderData;
	};

	class TOYGE_CORE_API RenderMesh
	{
	public:
		RenderMesh(const Ptr<Mesh> & mesh);

		const Ptr<RenderInput> & GetRenderInput() const
		{
			return _input;
		}

	private:
		Ptr<RenderInput> _input;
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