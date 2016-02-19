#pragma once
#ifndef D3D11RENDERBUFFER_H
#define D3D11RENDERBUFFER_H

#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\D3D11\D3D11ResourceView.h"

namespace ToyGE
{
	class D3D11BufferShaderResourceView : public BufferShaderResourceView, public D3D11ShaderResourceView
	{
	public:
		virtual ~D3D11BufferShaderResourceView() = default;

		virtual void Release() override
		{
			hardwareSRV = nullptr;
		}
	};

	class D3D11BufferUnorderedAccessView : public BufferUnorderedAccessView, public D3D11UnorderedAccessView
	{
	public:
		virtual ~D3D11BufferUnorderedAccessView() = default;

		virtual void Release() override
		{
			hardwareUAV = nullptr;
		}
	};

	class D3D11BufferRenderTargetView : public BufferRenderTargetView, public D3D11RenderTargetView
	{
	public:
		virtual ~D3D11BufferRenderTargetView() = default;

		virtual void Release() override
		{
			hardwareRTV = nullptr;
		}
	};

	class D3D11RenderBuffer : public virtual RenderBuffer
	{
	public:
		virtual void Init(const void * pInitData, bool bCacheData = false) override;

		virtual void Release() override
		{
			RenderBuffer::Release();
			_hardwareBuffer = nullptr;
		}

		RenderDataDesc Map(MapType mapFlag) override;

		void UnMap() override;

		bool CopyTo(const Ptr<RenderBuffer> & dst, int32_t dstBytesOffset, int32_t srcBytesOffset, int32_t cpyBytesSize) const override;

		void CopyStructureCountTo(
			const Ptr<RenderBuffer> & dst,
			uint32_t bytesOffset,
			int32_t firstElement,
			int32_t numElements,
			RenderFormat format,
			uint32_t uavFlag) override;

		const Ptr<ID3D11Buffer> & GetHardwareBuffer() const
		{
			return _hardwareBuffer;
		}

	protected:
		virtual Ptr<BufferShaderResourceView> CreateShaderResourceView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat) override;

		virtual Ptr<BufferUnorderedAccessView> CreateUnorderedAccessView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat, uint32_t uavFlags) override;

		virtual Ptr<BufferRenderTargetView> CreateRenderTargetView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat) override;

	private:
		Ptr<ID3D11Buffer> _hardwareBuffer;
	};

	class D3D11VertexBuffer : public virtual VertexBuffer, public virtual D3D11RenderBuffer
	{
	};

	class D3D11VertexInputLayout : public VertexInputLayout
	{
	public:
		Ptr<ID3D11InputLayout> hardwareInputLayout;

		virtual void Init() override;

		virtual void Release() override
		{
			VertexInputLayout::Release();
			hardwareInputLayout = nullptr;
		}

	protected:
	};
}

#endif