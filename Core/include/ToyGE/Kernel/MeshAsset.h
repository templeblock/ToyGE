#pragma once
#ifndef MESHASSET_H
#define MESHASSET_H

#include "ToyGE\Kernel\Asset.h"

namespace ToyGE
{
	enum class MeshVertexElementSignature
	{
		MVET_POSITION,
		MVET_TEXCOORD,
		MVET_NORMAL,
		MVET_TANGENT,
		MVET_BITANGENT,
		MVET_COLOR
	};

	struct TOYGE_CORE_API MeshVertexElementDesc
	{
		MeshVertexElementSignature signature;
		int32_t signatureIndex;
		int32_t bytesOffset;
		int32_t bytesSize;
	};

	struct TOYGE_CORE_API MeshVertexDesc
	{
		int32_t bytesSize;
		std::vector<MeshVertexElementDesc> elementsDesc;
	};

	class TOYGE_CORE_API MeshVertexSlotData
	{
	public:
		std::shared_ptr<uint8_t> rawBuffer;
		int32_t bufferSize;
		MeshVertexDesc vertexDesc;

		int32_t GetNumVertices() const
		{
			return bufferSize / vertexDesc.bytesSize;
		}

		int32_t FindVertexElement(MeshVertexElementSignature signature, int32_t signatureIndex)
		{
			int32_t index = 0;
			for (auto & elementDesc : vertexDesc.elementsDesc)
			{
				if (elementDesc.signature == signature && elementDesc.signatureIndex == signatureIndex)
					return index;
				++index;
			}
			return -1;
		}

		template<class ElementType>
		ElementType * GetElement(int32_t vertexIndex, MeshVertexElementSignature signature, int32_t signatureIndex)
		{
			auto elementIndex = FindVertexElement(signature, signatureIndex);
			if (elementIndex >= 0)
			{
				auto pElement = rawBuffer.get() + vertexIndex * vertexDesc.bytesSize + vertexDesc.elementsDesc[elementIndex].bytesOffset;
				return reinterpret_cast<ElementType*>(pElement);
			}
			return nullptr;
		}
	};

	class TOYGE_CORE_API MeshElement
	{
	public:
		std::vector< Ptr<MeshVertexSlotData> > vertexData;
		std::vector<uint32_t> indices;
		Ptr<class MaterialAsset> material;
	};

	class TOYGE_CORE_API MeshAsset : public Asset
	{
	public:
		virtual ~MeshAsset() = default;

		virtual void Load() override;

		virtual void Save() override;

		virtual void Init() override;

		virtual String GetAssetExtension() const override
		{
			return MeshAsset::GetExtension();
		}

		void SetData(const std::vector<Ptr<MeshElement>> & data) { _data = data; }
		const std::vector<Ptr<MeshElement>> & GetData() const
		{
			return _data;
		}

		CLASS_GET(Mesh, Ptr<class Mesh>, _mesh);

		static const String & GetExtension()
		{
			const static String ex = ".tmesh";
			return ex;
		}

		static Ptr<MeshAsset> New()
		{
			return std::shared_ptr<MeshAsset>(new MeshAsset);
		}

	private:
		std::vector<Ptr<MeshElement>> _data;
		Ptr<class Mesh> _mesh;

		void UpdateFromMesh();
	};

	TOYGE_CORE_API bool AssimpLoadMesh(const Ptr<MeshAsset> & asset, bool bFlipUV, bool bLeftHanded);
}

#endif
