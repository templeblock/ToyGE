#include "ToyGE\D3D11\D3D11Texture.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"

namespace ToyGE
{
	void D3D11Texture::GetD3DTextureCreateFlags(bool bWithInitData, uint32_t & d3dBindFlags, uint32_t & d3dCpuAccessFlags, uint32_t & d3dMiscFlags, D3D11_USAGE & d3dUsage)
	{
		d3dBindFlags = d3dCpuAccessFlags = d3dMiscFlags = 0;

		d3dUsage = D3D11_USAGE_DEFAULT;

		if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
		{
			if (bWithInitData)
				d3dUsage = D3D11_USAGE_IMMUTABLE;
			else
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with no initdata");
		}

		if (_desc.bindFlag & TEXTURE_BIND_SHADER_RESOURCE)
		{
			d3dBindFlags |= D3D11_BIND_SHADER_RESOURCE;
			//d3dUsage = D3D11_USAGE_DEFAULT;
		}

		if (_desc.bindFlag & TEXTURE_BIND_STREAM_OUTPUT)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_STREAM_OUTPUT");

			d3dBindFlags |= D3D11_BIND_STREAM_OUTPUT;
		}

		if (_desc.bindFlag & TEXTURE_BIND_RENDER_TARGET)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_RENDER_TARGET");

			d3dBindFlags |= D3D11_BIND_RENDER_TARGET;
		}

		if (_desc.bindFlag & TEXTURE_BIND_DEPTH_STENCIL)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_DEPTH_STENCIL");

			d3dBindFlags |= D3D11_BIND_DEPTH_STENCIL;
		}

		if (_desc.bindFlag & TEXTURE_BIND_UNORDERED_ACCESS)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_UNORDERED_ACCESS");

			d3dBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}


		if (_desc.bindFlag & TEXTURE_BIND_GENERATE_MIPS)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_GENERATE_MIPS");

			d3dBindFlags |= D3D11_BIND_RENDER_TARGET;
			d3dBindFlags |= D3D11_BIND_SHADER_RESOURCE;
			d3dMiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}


		if ((_desc.bindFlag & TEXTURE_BIND_IMMUTABLE) &&
			(_desc.cpuAccess & CPU_ACCESS_WRITE))
		{
			ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with CPU_ACCESS_WRITE");
		}


		if ((_desc.cpuAccess & CPU_ACCESS_READ) && (_desc.cpuAccess & CPU_ACCESS_WRITE))
		{
			d3dCpuAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			d3dUsage = D3D11_USAGE_STAGING;
		}
		else if (_desc.cpuAccess & CPU_ACCESS_READ)
		{
			d3dCpuAccessFlags = D3D11_CPU_ACCESS_READ;
			d3dUsage = D3D11_USAGE_STAGING;
		}
		else if (_desc.cpuAccess & CPU_ACCESS_WRITE)
		{
			d3dCpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
			d3dUsage = D3D11_USAGE_DYNAMIC;
		}

		if (_desc.bCube)
		{
			d3dMiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}
	}

	void D3D11Texture::GetFlagsFromD3D(uint32_t d3dBindFlags, uint32_t d3dCpuAccessFlags, uint32_t d3dMiscFlags, uint32_t & outBindFlags, uint32_t & outCpuAccess, uint32_t & bCube)
	{
		outBindFlags = 0;
		outCpuAccess = 0;
		bCube = false;

		if (d3dBindFlags & D3D11_BIND_SHADER_RESOURCE)
			outBindFlags |= TEXTURE_BIND_SHADER_RESOURCE;
		if (d3dBindFlags & D3D11_BIND_RENDER_TARGET)
			outBindFlags |= TEXTURE_BIND_RENDER_TARGET;
		if (d3dBindFlags & D3D11_BIND_DEPTH_STENCIL)
			outBindFlags |= TEXTURE_BIND_DEPTH_STENCIL;
		if (d3dBindFlags & D3D11_BIND_UNORDERED_ACCESS)
			outBindFlags |= TEXTURE_BIND_UNORDERED_ACCESS;
		if (d3dBindFlags & D3D11_BIND_STREAM_OUTPUT)
			outBindFlags |= TEXTURE_BIND_STREAM_OUTPUT;

		if (d3dMiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
			outBindFlags |= TEXTURE_BIND_GENERATE_MIPS;

		if (d3dMiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
			bCube = true;

		if (d3dCpuAccessFlags & D3D11_CPU_ACCESS_READ)
			outCpuAccess |= CPU_ACCESS_READ;
		if (d3dCpuAccessFlags & D3D11_CPU_ACCESS_WRITE)
			outCpuAccess |= CPU_ACCESS_WRITE;
	}

	bool D3D11Texture::CopyTo(const Ptr<Texture> & dst,
		int32_t dstMipLevel, int32_t dstArrayIndex, int32_t xOffset, int32_t yOffset, int32_t zOffset,
		int32_t srcMipLevel, int32_t srcArrayIndex, const std::shared_ptr<Box> & srcBox) const
	{
		if (dst->GetDesc().format != _desc.format || dst->GetDesc().bCube != _desc.bCube || dst->GetType() != _type)
			return false;

		auto & dstSize = dst->GetMipSize(dstMipLevel);
		auto & srcSize = dst->GetMipSize(dstMipLevel);

		int3 copySize;
		if (srcBox)
		{
			copySize.x() = srcBox->right - srcBox->left;
			copySize.y() = srcBox->bottom - srcBox->top;
			copySize.z() = srcBox->back - srcBox->front;
		}
		else
		{
			copySize.x() = srcSize.x();
			copySize.y() = srcSize.y();
			copySize.z() = srcSize.z();
		}

		if (   xOffset + copySize.x() > dstSize.x()
			|| yOffset + copySize.y() > dstSize.y()
			|| zOffset + copySize.z() > dstSize.z())
			return false;

		/*if ( dst->GetMipSize(dstMipLevel) != GetMipSize(srcMipLevel) )
			return false;*/

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		Ptr<D3D11Texture> d3dDst = std::static_pointer_cast<D3D11Texture>(dst);

		auto dstSubIndex = ::D3D11CalcSubresource(
			dstMipLevel,
			dstArrayIndex,
			dst->GetDesc().mipLevels);

		auto srcSubIndex = ::D3D11CalcSubresource(
			srcMipLevel,
			srcArrayIndex,
			_desc.mipLevels);

		if (srcBox)
		{
			D3D11_BOX d3dSrcBox;
			d3dSrcBox.left = srcBox->left;
			d3dSrcBox.right = srcBox->right;
			d3dSrcBox.top = srcBox->top;
			d3dSrcBox.bottom = srcBox->bottom;
			d3dSrcBox.front = srcBox->front;
			d3dSrcBox.back = srcBox->back;

			D3D11RenderEngine::d3d11DeviceContext->CopySubresourceRegion(
				d3dDst->GetHardwareTexture().get(),
				dstSubIndex,
				xOffset,
				yOffset,
				zOffset,
				GetHardwareTexture().get(),
				srcSubIndex,
				&d3dSrcBox);
		}
		else
		{ 
			D3D11RenderEngine::d3d11DeviceContext->CopySubresourceRegion(
				d3dDst->GetHardwareTexture().get(),
				dstSubIndex,
				xOffset,
				yOffset,
				zOffset,
				GetHardwareTexture().get(),
				srcSubIndex,
				nullptr);
		}

		return true;
	}

	RenderDataDesc D3D11Texture::Map(MapType mapFlag, int32_t mipLevel, int32_t arrayIndex)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		uint32_t subIndex = ::D3D11CalcSubresource(
			mipLevel,
			arrayIndex,
			_desc.mipLevels);
		auto rc = D3D11RenderEngine::d3d11DeviceContext;
		auto rs = GetHardwareTexture().get();
		rc->Map(rs, subIndex, static_cast<D3D11_MAP>(mapFlag), 0, &mappedResource);

		RenderDataDesc ret;
		ret.pData = mappedResource.pData;
		ret.rowPitch = mappedResource.RowPitch;
		ret.slicePitch = mappedResource.DepthPitch;

		_mappedSubResource = static_cast<int32_t>(subIndex);

		if (mapFlag != MapType::MAP_READ)
			_bDirty = true;

		return ret;
	}

	void D3D11Texture::UnMap()
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		D3D11RenderEngine::d3d11DeviceContext->Unmap(GetHardwareTexture().get(), static_cast<uint32_t>(_mappedSubResource));
	}

	bool D3D11Texture::GenerateMips()
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>( Global::GetRenderEngine() );

		if (_desc.bindFlag & TEXTURE_BIND_GENERATE_MIPS)
		{
			auto srv = GetShaderResourceView(0, _desc.mipLevels, 0, _desc.arraySize, !!_desc.bCube);

			auto d3d11SRV = std::static_pointer_cast<D3D11TextureShaderResourceView>(srv);

			D3D11RenderEngine::d3d11DeviceContext->GenerateMips(d3d11SRV->hardwareSRV.get());

			_bDirty = true;

			return true;
		}

		return false;
	}
}