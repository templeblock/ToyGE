#pragma once
#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include "ToyGE\RenderEngine\RenderResource.h"

namespace ToyGE
{
	/**
	* Buffer view
	*/
	class TOYGE_CORE_API BufferResourceView
	{
	public:
		int32_t firstElement;
		int32_t numElements;

		BufferResourceView()
			: firstElement(0),
			numElements(0)
		{

		}

		virtual ~BufferResourceView() = default;
	};

	class TOYGE_CORE_API BufferShaderResourceView : public ShaderResourceView, public BufferResourceView
	{
	public:
		virtual ~BufferShaderResourceView() = default;
	};

	class TOYGE_CORE_API BufferUnorderedAccessView : public UnorderedAccessView, public BufferResourceView
	{
	public:
		uint32_t uavFlags = 0;

		virtual ~BufferUnorderedAccessView() = default;
	};

	class TOYGE_CORE_API BufferRenderTargetView : public RenderTargetView, public BufferResourceView
	{
	public:
		virtual ~BufferRenderTargetView() = default;
	};


	/**
	* Buffer
	*/

	struct RenderBufferDesc
	{
		int32_t		elementSize = 0;
		int32_t		numElements = 0;
		uint32_t	bindFlag = 0;
		uint32_t	cpuAccess = 0;
		uint32_t	bStructured = 0;
	};

	class TOYGE_CORE_API RenderBuffer : public RenderResource
	{
	public:
		RenderBuffer() : RenderResource(RenderResourceType::RRT_BUFFER) {};

		virtual ~RenderBuffer() = default;

		virtual void Init(const void * pInitData, bool bCacheData = false);

		virtual void Init() { Init(nullptr); }

		virtual void Release() override;

		void SetDesc(const RenderBufferDesc & desc)
		{
			_desc = desc;
			_bDirty = true;
			_bInit = false;
		}
		CLASS_GET(Desc, RenderBufferDesc, _desc);

		int32_t GetDataSize() const
		{
			return _desc.elementSize * _desc.numElements;
		}

		bool Dump(void * outDumpBuffer);

		virtual RenderDataDesc Map(MapType mapFlag) = 0;

		virtual void UnMap() = 0;

		virtual bool CopyTo(const Ptr<RenderBuffer> & dst, int32_t dstBytesOffset, int32_t srcBytesOffset, int32_t cpyBytesSize) const = 0;

		virtual void CopyStructureCountTo(
			const Ptr<RenderBuffer> & dst,
			uint32_t bytesOffset,
			int32_t firstElement,
			int32_t numElements,
			RenderFormat format,
			uint32_t uavFlag) = 0;

		Ptr<BufferShaderResourceView> GetShaderResourceView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat);

		Ptr<BufferUnorderedAccessView> GetUnorderedAccessView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat, uint32_t uavFlags);

		Ptr<BufferRenderTargetView> GetRenderTargetView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat);

		CLASS_SET(Dirty, bool, _bDirty);
		bool IsDirty() const
		{
			return _bDirty;
		}

	protected:
		RenderBufferDesc _desc;
		std::shared_ptr<uint8_t> _cachedData;

		bool _bDirty = false;

		// Cache views
		std::vector<Ptr<BufferShaderResourceView>>	_srvCache;
		std::vector<Ptr<BufferUnorderedAccessView>> _uavCache;
		std::vector<Ptr<BufferRenderTargetView>>	_rtvCache;

		virtual Ptr<BufferShaderResourceView> CreateShaderResourceView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat) = 0;

		virtual Ptr<BufferUnorderedAccessView> CreateUnorderedAccessView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat, uint32_t uavFlags) = 0;

		virtual Ptr<BufferRenderTargetView> CreateRenderTargetView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat) = 0;
	};


	// Vertex buffer
	enum VertexBufferType
	{
		VERTEX_BUFFER_GEOMETRY,
		VERTEX_BUFFER_INSTANCE
	};

	struct VertexElementDesc
	{
		String			name;
		int32_t			index;
		RenderFormat	format;
		int32_t			instanceDataRate;
		int32_t			bytesOffset;
		int32_t			bytesSize;

		bool operator==(const VertexElementDesc & rhs) const
		{
			return name == rhs.name &&
				index == rhs.index &&
				format == rhs.format &&
				instanceDataRate == rhs.instanceDataRate &&
				bytesOffset == rhs.bytesOffset &&
				bytesSize == rhs.bytesSize;
		}
	};

	struct VertexInputSlotDesc
	{
		std::vector<VertexElementDesc> vertexElementsDesc;
		VertexBufferType type;

		// Sort elements by its bytesOffset
		void Sort()
		{
			std::sort(vertexElementsDesc.begin(), vertexElementsDesc.end(), 
				[](const VertexElementDesc & lhs, const VertexElementDesc & rhs) -> bool
			{
				return lhs.bytesOffset < rhs.bytesOffset;
			});
		}

		bool operator==(const VertexInputSlotDesc & rhs) const
		{
			return vertexElementsDesc == rhs.vertexElementsDesc &&
				type == rhs.type;
		}
	};

	class TOYGE_CORE_API VertexBuffer : public virtual RenderBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		CLASS_SET(Type, VertexBufferType, _type);
		CLASS_GET(Type, VertexBufferType, _type);

		CLASS_SET(ElementsDesc, std::vector<VertexElementDesc>, _vertexElementsDesc);
		CLASS_GET(ElementsDesc, std::vector<VertexElementDesc>, _vertexElementsDesc);

	protected:
		VertexBufferType _type;
		std::vector<VertexElementDesc> _vertexElementsDesc;
	};


	class ShaderProgram;

	class TOYGE_CORE_API VertexInputLayout : public RenderResource
	{
		typedef std::vector<Ptr<VertexInputLayout>> VertexInputLayoutList;

	public:
		VertexInputLayout() : RenderResource(RenderResourceType::RRT_VERTEXINPUTLAYOUT) {}

		virtual ~VertexInputLayout() = default;

		CLASS_GET(VertexShader, Ptr<ShaderProgram>, _vertexShader);

		CLASS_GET(InputSlotsDesc, std::vector<VertexInputSlotDesc>, _inputSlotsDesc);

		static Ptr<VertexInputLayout> GetVertexInputLayout(const Ptr<ShaderProgram> & vertexShader, const std::vector<Ptr<VertexBuffer>> & vertexBuffers);

	protected:
		Ptr<ShaderProgram> _vertexShader;
		std::vector<VertexInputSlotDesc> _inputSlotsDesc;

		static std::map<Ptr<ShaderProgram>, VertexInputLayoutList> _vertexInputLayoutMap;
	};

	class TOYGE_CORE_API VertexBufferBuilder
	{
	public:
		VertexBufferType type = VertexBufferType::VERTEX_BUFFER_GEOMETRY;
		std::vector<VertexElementDesc> vertexElementsDesc;
		int32_t vertexSize = 0;
		std::shared_ptr<uint8_t> dataBuffer;
		uint8_t * pData;
		int32_t numVertices;

		void AddElementDesc(const String & name, int32_t index, RenderFormat format, int32_t instanceRate);

		void SetNumVertices(int32_t numVertices);

		template <typename T>
		void Add(const T & value)
		{
			::memcpy(pData, &value, sizeof(value));
			pData += sizeof(value);
		}

		Ptr<VertexBuffer> Finish();
	};
}

#endif
