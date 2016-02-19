#include "ToyGE\Kernel\TextureAsset.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

#include <DirectXTex.h>
#include <FreeImage.h>

namespace ToyGE
{
	void ImageToDXImage(const Ptr<TextureAsset> & image, std::vector<DirectX::Image> & outImgs, DirectX::TexMetadata & outMetaData)
	{
		using namespace DirectX;

		//std::vector<DirectX::Image> dxImages;

		int32_t dataIndex = 0;
		int32_t adjustArraySize = image->GetArraySize();
		if (image->IsCube())
			adjustArraySize *= 6;

		for (int32_t arrayIndex = 0; arrayIndex < adjustArraySize; ++arrayIndex)
		{
			int32_t w = image->GetWidth();
			int32_t h = image->GetHeight();
			int32_t d = image->GetDepth();
			for (int32_t mipLevel = 0; mipLevel < image->GetMipLevels(); ++mipLevel)
			{
				const auto & dataDesc = image->GetDataDescs()[dataIndex++];

				DirectX::Image dxImage;
				dxImage.format = static_cast<DXGI_FORMAT>(image->GetFormat());
				dxImage.width = static_cast<size_t>(w);
				dxImage.height = static_cast<size_t>(h);
				dxImage.rowPitch = static_cast<size_t>(dataDesc.rowPitch);
				dxImage.slicePitch = static_cast<size_t>(dataDesc.slicePitch);
				dxImage.pixels = static_cast<uint8_t*>(dataDesc.pData);

				outImgs.push_back(dxImage);

				w = std::max<int32_t>(1, w >> 1);
				h = std::max<int32_t>(1, h >> 1);
				d = std::max<int32_t>(1, d >> 1);
			}
		}

		//TexMetadata texData;
		outMetaData.width = image->GetWidth();
		outMetaData.height = image->GetHeight();
		outMetaData.depth = image->GetDepth();
		outMetaData.arraySize = adjustArraySize;
		outMetaData.mipLevels = image->GetMipLevels();
		switch (image->GetType())
		{
		case TEXTURE_1D:
			outMetaData.dimension = TEX_DIMENSION_TEXTURE1D;
			break;
		case TEXTURE_2D:
			outMetaData.dimension = TEX_DIMENSION_TEXTURE2D;
			break;
		case TEXTURE_3D:
			outMetaData.dimension = TEX_DIMENSION_TEXTURE3D;
			break;
		default:
			break;
		}
		outMetaData.format = static_cast<DXGI_FORMAT>(image->GetFormat());
		outMetaData.miscFlags = 0;
		if (image->IsCube())
			outMetaData.miscFlags |= TEX_MISC_TEXTURECUBE;
		outMetaData.miscFlags2 = 0;
	}

	void DXImageToImage(const DirectX::ScratchImage & scratchImage, const Ptr<TextureAsset> & outImage)
	{
		using namespace DirectX;

		TexMetadata metaData = scratchImage.GetMetadata();

		outImage->SetWidth(static_cast<int32_t>(metaData.width));
		outImage->SetHeight(static_cast<int32_t>(metaData.height));
		outImage->SetDepth(static_cast<int32_t>(metaData.depth));
		outImage->SetArraySize(static_cast<int32_t>(metaData.arraySize));
		outImage->SetMipLevels(static_cast<int32_t>(metaData.mipLevels));
		outImage->SetFormat(static_cast<RenderFormat>(metaData.format));
		switch (metaData.dimension)
		{
		case TEX_DIMENSION_TEXTURE1D:
			outImage->SetType(TEXTURE_1D);
			break;
		case TEX_DIMENSION_TEXTURE2D:
			outImage->SetType(TEXTURE_2D);
			break;
		case TEX_DIMENSION_TEXTURE3D:
			outImage->SetType(TEXTURE_3D);
			break;
		default:
			break;
		}
		if (metaData.miscFlags & TEX_MISC_TEXTURECUBE)
		{
			outImage->SetCube(true);
			outImage->SetArraySize(outImage->GetArraySize() / 6);
		}

		auto imgs = scratchImage.GetImages();
		auto numImgs = scratchImage.GetImageCount();
		size_t imgDataSize = 0;
		for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel)
		{
			auto img = scratchImage.GetImage(mipLevel, 0, 0);
			imgDataSize += img->slicePitch;
		}
		imgDataSize *= metaData.arraySize;
		auto imageData = MakeBufferedDataShared(imgDataSize);
		outImage->SetData( imageData );

		uint8_t * pDst = imageData.get();
		std::vector<RenderDataDesc> dataDescs;

		for (size_t arrayIndex = 0; arrayIndex < metaData.arraySize; ++arrayIndex)
		{
			for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel)
			{
				RenderDataDesc dataDesc;
				dataDesc.pData = pDst;
				dataDesc.rowPitch = scratchImage.GetImage(mipLevel, arrayIndex, 0)->rowPitch;
				dataDesc.slicePitch = scratchImage.GetImage(mipLevel, arrayIndex, 0)->slicePitch;
				dataDescs.push_back(dataDesc);

				for (size_t slice = 0; slice < metaData.depth; ++slice)
				{
					auto img = scratchImage.GetImage(mipLevel, arrayIndex, slice);
					memcpy(pDst, img->pixels, img->slicePitch);
					pDst += img->slicePitch;
				}
			}
		}
		outImage->SetDataDescs(dataDescs);
	}

	bool LoadDDSImage(const Ptr<TextureAsset> & image)
	{
		using namespace DirectX;

		TexMetadata metaData;
		ScratchImage scractchImage;
		auto u16Path = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>( image->GetLoadPath() );
		bool bSuccess = (LoadFromDDSFile(reinterpret_cast<const wchar_t*>( u16Path.c_str() ), DDS_FLAGS_NO_LEGACY_EXPANSION, &metaData, scractchImage) == S_OK);

		if (!bSuccess)
			return bSuccess;

		DXImageToImage(scractchImage, image);

		return true;
	}

	void SaveDDSImage(const Ptr<TextureAsset> & image)
	{
		using namespace DirectX;

		std::vector<DirectX::Image> dxImages;
		TexMetadata metaData;
		ImageToDXImage(image, dxImages, metaData);

		auto u16Path = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(image->GetSavePath());
		SaveToDDSFile(&dxImages[0], dxImages.size(), metaData, DDS_FLAGS_NONE, reinterpret_cast<const wchar_t*>(u16Path.c_str()));
	}

	void Compress(const Ptr<TextureAsset> & image, RenderFormat compressFormat)
	{
		using namespace DirectX;

		if (IsCompress(image->GetFormat()) || !IsCompress(compressFormat))
			return;

		std::vector<DirectX::Image> dxImages;
		TexMetadata metaData;
		ImageToDXImage(image, dxImages, metaData);

		ScratchImage compressedScratchImage;
		DirectX::Compress(&dxImages[0], dxImages.size(), metaData, static_cast<DXGI_FORMAT>(compressFormat), 0, 0.0f, compressedScratchImage);

		DXImageToImage(compressedScratchImage, image);
	}

	void Decompress(const Ptr<TextureAsset> & image, RenderFormat format)
	{
		using namespace DirectX;

		if (!IsCompress(image->GetFormat()))
			return;

		std::vector<DirectX::Image> dxImages;
		TexMetadata metaData;
		ImageToDXImage(image, dxImages, metaData);

		ScratchImage decompressedScratchImage;
		DirectX::Decompress(&dxImages[0], dxImages.size(), metaData, static_cast<DXGI_FORMAT>(format), decompressedScratchImage);

		DXImageToImage(decompressedScratchImage, image);
	}

	bool LoadCommonImage(const Ptr<TextureAsset> & asset)
	{
		auto file = Global::GetPlatform()->CreatePlatformFile(asset->GetLoadPath(), FILE_OPEN_READ);
		if (!file->IsValid())
			return false;

		auto imgFmt = FreeImage_GetFileType(asset->GetLoadPath().c_str());
		if (FIF_UNKNOWN == imgFmt)
			return false;
		auto *pImg = FreeImage_Load(imgFmt, asset->GetLoadPath().c_str());
		if (nullptr == pImg)
			return false;
		auto *imgInfo = FreeImage_GetInfo(pImg);
		if (nullptr == pImg)
			return false;

		auto pitch = FreeImage_GetPitch(pImg);
		auto imgType = FreeImage_GetImageType(pImg);
		std::unique_ptr<uint8_t[]> buffer;
		int32_t index = 0;
		if (FIT_BITMAP == imgType)
		{
			auto bpp = FreeImage_GetBPP(pImg);
			uint8_t *pBytes = (uint8_t*)(FreeImage_GetBits(pImg));
			int32_t bufferBytesPerPixel = 0;
			if (8 == bpp)
			{
				bufferBytesPerPixel = 1;
				buffer.reset(new uint8_t[imgInfo->bmiHeader.biWidth * imgInfo->bmiHeader.biHeight]);
				for (int y = 0; y != imgInfo->bmiHeader.biHeight; ++y)
				{
					for (int x = 0; x != imgInfo->bmiHeader.biWidth; ++x)
					{
						buffer[index++] = *pBytes;
						pBytes += 1;
					}
				}
				asset->SetFormat(RENDER_FORMAT_R8_UNORM);
			}
			else
			{
				bufferBytesPerPixel = 4;
				buffer.reset(new uint8_t[imgInfo->bmiHeader.biWidth * imgInfo->bmiHeader.biHeight * bufferBytesPerPixel]);
				for (int y = 0; y != imgInfo->bmiHeader.biHeight; ++y)
				{
					for (int x = 0; x != imgInfo->bmiHeader.biWidth; ++x)
					{
						RGBQUAD rgbQuad;
						FreeImage_GetPixelColor(pImg, x, y, &rgbQuad);
						buffer[index++] = rgbQuad.rgbRed;
						buffer[index++] = rgbQuad.rgbGreen;
						buffer[index++] = rgbQuad.rgbBlue;
						buffer[index++] = 255;
						pBytes += bpp / 8;
					}
				}
				asset->SetFormat(RENDER_FORMAT_R8G8B8A8_UNORM);
			}

			asset->SetWidth(static_cast<int32_t>(imgInfo->bmiHeader.biWidth));
			asset->SetHeight(static_cast<int32_t>(imgInfo->bmiHeader.biHeight));
			asset->SetDepth(static_cast<int32_t>(imgInfo->bmiHeader.biPlanes));
			asset->SetType(TEXTURE_2D);

			RenderDataDesc renderData;
			renderData.pData = &buffer[0];
			renderData.rowPitch = asset->GetWidth() * bufferBytesPerPixel;
			renderData.slicePitch = asset->GetHeight() * renderData.rowPitch;
			asset->SetDataDescs({ renderData });
			asset->SetArraySize(1);
			asset->SetMipLevels(1);
			asset->SetData( std::shared_ptr<uint8_t>(buffer.release(), [](uint8_t *p) {delete[] p; }) );

			return true;
		}
		else
		{
			return false;
		}
	}

	void TextureAsset::Load()
	{
		if (_path.size() == 0)
		{
			ToyGE_LOG(LT_ERROR, "Try to load texture with empty path!");
			return;
		}

		auto extension = _path.substr( _path.rfind('.') );
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		if (extension == ".dds")
		{
			if (!LoadDDSImage(shared_from_this()->Cast<TextureAsset>()))
			{
				ToyGE_LOG(LT_WARNING, "Cannot load texture! %s", GetLoadPath().c_str());
				return;
			}
		}
		else
		{
			if (!LoadCommonImage(shared_from_this()->Cast<TextureAsset>()))
			{
				ToyGE_LOG(LT_WARNING, "Cannot load texture! %s", GetLoadPath().c_str());
				return;
			}
		}
		_bLoaded = true;
		_bDirty = false;

		ToyGE_LOG(LT_INFO, "Texture loaded! %s", GetLoadPath().c_str());
	}

	void TextureAsset::Init()
	{
		_texture = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(_type);
		_texture->SetAsset(shared_from_this()->Cast<TextureAsset>());

		TextureDesc texDesc;
		texDesc.width = _width;
		texDesc.height = _height;
		texDesc.depth = _depth;
		texDesc.arraySize = _arraySize;
		texDesc.mipLevels = _mipLevels;
		texDesc.bCube = _bCube;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.cpuAccess = 0;
		texDesc.format = _format;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		_texture->SetDesc(texDesc);
		_texture->Init(_dataDescs);

		_texture->_bDirty = false;

		_bInit = true;
	}

	void TextureAsset::Save()
	{
		if (_path.size() == 0)
		{
			ToyGE_LOG(LT_ERROR, "Try to save texture with empty path!");
			return;
		}

		if (_texture && _texture->_bDirty)
		{
			UpdateFromTexture();
		}

		SaveDDSImage(shared_from_this()->Cast<TextureAsset>());
		_bDirty = false;

		ToyGE_LOG(LT_INFO, "Texture saved! %s", GetSavePath().c_str());
	}

	void TextureAsset::Compress(RenderFormat format)
	{
		ToyGE::Compress(shared_from_this()->Cast<TextureAsset>(), format);
		_bDirty = true;
	}

	void TextureAsset::Decompress(RenderFormat format)
	{
		ToyGE::Decompress(shared_from_this()->Cast<TextureAsset>(), format);
		_bDirty = true;
	}

	void TextureAsset::UpdateFromTexture()
	{
		_data.reset();
		_dataDescs.clear();

		auto & desc = _texture->GetDesc();

		_type = _texture->GetType();
		_width = desc.width;
		_height = desc.height;
		_depth = desc.depth;
		_arraySize = desc.arraySize;
		_mipLevels = desc.mipLevels;
		_format = desc.format;
		_bCube = !!desc.bCube;

		_data = MakeBufferedDataShared(_texture->GetDataSize());
		_texture->Dump(_data.get(), _dataDescs);

		//int32_t adjustArraySize = _arraySize;
		//if (_bCube)
		//	adjustArraySize *= 6;

		//// Dump texture
		//auto dumpTexDesc = desc;
		//dumpTexDesc.bindFlag = 0;
		//dumpTexDesc.cpuAccess = CPU_ACCESS_READ;
		//auto dumpTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(_type);
		//dumpTex->SetDesc(dumpTexDesc);
		//dumpTex->Init();

		//for (int32_t arrayIndex = 0; arrayIndex < adjustArraySize; ++arrayIndex)
		//{
		//	for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
		//	{
		//		_texture->CopyTo(dumpTex, mipLevel, arrayIndex, 0, 0, 0, mipLevel, arrayIndex);
		//	}
		//}

		//// Creat buffer
		//int32_t pixelBits = GetRenderFormatNumBits(dumpTexDesc.format);
		//bool bCompression = IsCompress(dumpTexDesc.format);

		//int32_t dataSize = 0;
		//for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
		//{
		//	auto mipSize = dumpTex->GetMipSize(mipLevel);
		//	int32_t w = mipSize.x;
		//	int32_t h = mipSize.y;
		//	int32_t d = mipSize.z;

		//	if (bCompression)
		//	{
		//		w = (w + 3) / 4 * 4;
		//		h = (h + 3) / 4 * 4;

		//		auto pitch = (16 * pixelBits / 8) * w / 4;
		//		auto slice = pitch * h / 4;
		//		dataSize += slice * d;
		//	}
		//	else
		//	{
		//		auto pitch = pixelBits / 8 * w;
		//		auto slice = pitch * h;
		//		dataSize += slice * d;
		//	}
		//}
		//dataSize *= adjustArraySize;
		//_data = MakeBufferedDataShared(dataSize);

		//// Copy from mapped data
		//uint8_t * pDst = static_cast<uint8_t*>(_data.get());

		//for (int32_t arrayIndex = 0; arrayIndex < adjustArraySize; ++arrayIndex)
		//{
		//	for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
		//	{
		//		auto mipSize = dumpTex->GetMipSize(mipLevel);
		//		int32_t w = mipSize.x;
		//		int32_t h = mipSize.y;
		//		int32_t d = mipSize.z;

		//		int32_t cpyPitch = 0;
		//		int32_t numCpys = 0;

		//		if (bCompression)
		//		{
		//			w = (w + 3) / 4 * 4;
		//			h = (h + 3) / 4 * 4;

		//			cpyPitch = (16 * pixelBits / 8) * (w / 4);
		//			numCpys = h / 4;
		//		}
		//		else
		//		{
		//			cpyPitch = pixelBits / 8 * w;
		//			numCpys = h;
		//		}

		//		auto mappedData = dumpTex->Map(MAP_READ, mipLevel, arrayIndex);

		//		RenderDataDesc dataDesc;
		//		dataDesc.pData = pDst;
		//		dataDesc.rowPitch = cpyPitch;
		//		dataDesc.slicePitch = dataDesc.rowPitch * numCpys;
		//		_dataDescs.push_back(dataDesc);

		//		for (int32_t slice = 0; slice < d; ++slice)
		//		{
		//			uint8_t * pSrc = static_cast<uint8_t*>(mappedData.pData) + slice * mappedData.slicePitch;
		//			for (int32_t row = 0; row < numCpys; ++row)
		//			{
		//				memcpy(pDst, pSrc, cpyPitch);
		//				pSrc += mappedData.rowPitch;
		//				pDst += cpyPitch;
		//			}
		//		}

		//		dumpTex->UnMap();
		//	}
		//}
	}
}