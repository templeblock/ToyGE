#include "ToyGE\D3D11\D3D11TextureCube.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11Util.h"

namespace ToyGE
{
	static inline
		TextureDesc TransDesc2D(const TextureDesc & desc)
	{
		TextureDesc descTrans = desc;
		descTrans.arraySize *= 6;
		return descTrans;
	}

	D3D11TextureCube::D3D11TextureCube(const TextureDesc & desc)
		: D3D11TextureCube(desc, std::vector<RenderDataDesc>())
	{

	}

	D3D11TextureCube::D3D11TextureCube(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList)
		: D3D11Texture2D(TransDesc2D(desc), initDataList)
	{
		_desc.arraySize /= 6;
	}

	std::shared_ptr<D3D11TextureCube>
		D3D11TextureCube::CreateFromRawD3D(
		const Ptr<ID3D11Device> & rawDevice,
		const Ptr<ID3D11Texture2D> & rawTextureCube)
	{
		return std::static_pointer_cast<D3D11TextureCube>( D3D11Texture2D::CreateFromRawD3D(rawDevice, rawTextureCube) );
	}

	const Ptr<ID3D11ShaderResourceView>& 
		D3D11TextureCube::AcquireRawD3DShaderResourceView_Cube(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstFaceOffset, int32_t numCubes, RenderFormat formatHint)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (numCubes > 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
			desc.TextureCubeArray.MostDetailedMip = firstMipLevel;
			desc.TextureCubeArray.MipLevels = numMipLevels;
			desc.TextureCubeArray.First2DArrayFace = firstFaceOffset;
			desc.TextureCubeArray.NumCubes = numCubes;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			desc.TextureCube.MostDetailedMip = firstMipLevel;
			desc.TextureCube.MipLevels = numMipLevels;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitShaderResourceView(desc);
	}

	//const Ptr<ID3D11RenderTargetView>& 
	//	D3D11TextureCube::AcquireRawD3DRenderTargetView_Cube(int mipLevel, int cubeIndex, CubeFace face, int numFaces)
	//{
	//	return D3D11Texture2D::AcquireRawD3DRenderTargetView(mipLevel, cubeIndex * 6 + face, numFaces);
	//}

	//const Ptr<ID3D11DepthStencilView>& 
	//	D3D11TextureCube::AcquireRawD3DDepthStencilView_Cube(int mipLevel, int cubeIndex, CubeFace face, int numFaces)
	//{
	//	return D3D11Texture2D::AcquireRawD3DDepthStencilView(mipLevel, cubeIndex * 6 + face, numFaces);
	//}

	//const Ptr<ID3D11UnorderedAccessView>& 
	//	D3D11TextureCube::AcquireRawD3DUnorderedAccessView_Cube(int mipLevel, int cubeIndex, CubeFace face, int numFaces)
	//{
	//	return D3D11Texture2D::AcquireRawD3DUnorderedAccessView(mipLevel, cubeIndex * 6 + face, numFaces);
	//}

}