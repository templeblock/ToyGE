#pragma once
#ifndef D3D11RENDERBUFFER_H
#define D3D11RENDERBUFFER_H

#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class D3D11RenderBuffer : public RenderBuffer
	{
	public:
		D3D11RenderBuffer(const RenderBufferDesc & desc, const void *pInitData);
		//D3D11RenderBuffer(const RenderBufferDesc & desc, const void *pInitData,
		//	VertexBufferType vertexBufferType, const std::vector<VertexElementDesc> & vertexElementDesc);

		RenderDataDesc Map(MapType mapFlag) override;

		void UnMap() override;

		void CopyStructureCountTo(
			const Ptr<RenderBuffer> & dst,
			uint32_t bytesOffset,
			int32_t firstElement,
			int32_t numElements,
			RenderFormat format,
			uint32_t uavFlag) override;

		bool CopyTo(const Ptr<RenderBuffer> & dst, int32_t dstBytesOffset, int32_t srcBytesOffset, int32_t cpyBytesSize) const override;

		const Ptr<ID3D11Buffer> & RawD3DBuffer() const
		{
			return _rawD3DBuffer;
		}

		const Ptr<ID3D11ShaderResourceView>&
			AcquireRawD3DShaderResourceView
			(int32_t firstElement, int32_t numElements, RenderFormat format);

		const Ptr<ID3D11RenderTargetView>&
			AcquireRawD3DRenderTargetView
			(int32_t firstElement, int32_t numElements, RenderFormat format);

		const Ptr<ID3D11UnorderedAccessView>&
			AcquireRawD3DUnorderedAccessView
			(int32_t firstElement, int32_t numElements, RenderFormat format, uint32_t uavFlags);


	private:
		Ptr<ID3D11Buffer> _rawD3DBuffer;
		std::map<uint64_t, Ptr<ID3D11ShaderResourceView>>	_srvMap;
		std::map<uint64_t, Ptr<ID3D11RenderTargetView>>		_rtvMap;
		std::map<uint64_t, Ptr<ID3D11UnorderedAccessView>>	_uavMap;

		void Init(const RenderBufferDesc & createDesc, const void *pInitData);

		void CreateBufferDesc(bool hasInitData, D3D11_BUFFER_DESC & outDesc);

		void ExtractD3DBindFlags(
			bool hasInitData,
			uint32_t & d3dBindFlags,
			uint32_t & d3dCpuAccessFlags,
			D3D11_USAGE & d3dUsage,
			uint32_t & d3dMiscFlags);
	};
}

#endif