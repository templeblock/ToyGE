#pragma once
#ifndef D3D11TEXTURECUBE_H
#define D3D11TEXTURECUBE_H

#include "ToyGE\D3D11\D3D11Texture2D.h"

namespace ToyGE
{
	class D3D11TextureCube : public D3D11Texture2D
	{
	public:
		D3D11TextureCube(const TextureDesc & desc);
		D3D11TextureCube(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList);

		static Ptr<D3D11TextureCube>
			CreateFromRawD3D(
			const Ptr<ID3D11Device> & rawDevice, const Ptr<ID3D11Texture2D> & rawTextureCube);

		const Ptr<ID3D11ShaderResourceView>& 
			AcquireRawD3DShaderResourceView_Cube
			(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstFaceOffset, int32_t numCubes, RenderFormat formatHint) override;

		//const Ptr<ID3D11RenderTargetView>& 
		//	AcquireRawD3DRenderTargetView_Cube
		//	(int mipLevel = 0, int cubeIndex = 0, CubeFace face = CUBE_FACE_X_POSITIVE, int numFaces = 1) override;

		//const Ptr<ID3D11DepthStencilView>& 
		//	AcquireRawD3DDepthStencilView_Cube
		//	(int mipLevel = 0, int cubeIndex = 0, CubeFace face = CUBE_FACE_X_POSITIVE, int numFaces = 1) override;

		//const Ptr<ID3D11UnorderedAccessView>& 
		//	AcquireRawD3DUnorderedAccessView_Cube
		//	(int mipLevel = 0, int cubeIndex = 0, CubeFace face = CUBE_FACE_X_POSITIVE, int numFaces = 1) override;
	};
}

#endif