#include "ToyGE\Kernel\ImageHelper.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Image.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\Kernel\Assert.h"

#include "DirectXTex.h"

namespace ToyGE
{
	namespace
	{
		static void ImageToDXImage(const Ptr<Image> & image, std::vector<DirectX::Image> & outImgs, DirectX::TexMetadata & outMetaData)
		{
			using namespace DirectX;

			//std::vector<DirectX::Image> dxImages;

			int32_t dataIndex = 0;
			int32_t adjustArraySize = image->GetArraySize();
			if (image->GetType() == TEXTURE_CUBE)
				adjustArraySize *= 6;

			for (int32_t arrayIndex = 0; arrayIndex < adjustArraySize; ++arrayIndex)
			{
				int32_t w = image->GetWidth();
				int32_t h = image->GetHeight();
				int32_t d = image->GetDepth();
				for (int32_t mipLevel = 0; mipLevel < image->GetMipLevels(); ++mipLevel)
				{
					const auto & dataDesc = image->DataDescs()[dataIndex++];

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
			case TEXTURE_CUBE:
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
			if (image->GetType() == TEXTURE_CUBE)
				outMetaData.miscFlags |= TEX_MISC_TEXTURECUBE;
			outMetaData.miscFlags2 = 0;
		}

		void DXImageToImage(const DirectX::ScratchImage & scratchImage, const Ptr<Image> & outImage)
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
				outImage->SetType(TEXTURE_CUBE);
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
			outImage->Data() = MakeBufferedDataShared(imgDataSize);

			uint8_t * pDst = outImage->Data().get();

			for (size_t arrayIndex = 0; arrayIndex < metaData.arraySize; ++arrayIndex)
			{
				for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel)
				{
					RenderDataDesc dataDesc;
					dataDesc.pData = pDst;
					dataDesc.rowPitch = scratchImage.GetImage(mipLevel, arrayIndex, 0)->rowPitch;
					dataDesc.slicePitch = scratchImage.GetImage(mipLevel, arrayIndex, 0)->slicePitch;
					outImage->DataDescs().push_back(dataDesc);

					for (size_t slice = 0; slice < metaData.depth; ++slice)
					{
						auto img = scratchImage.GetImage(mipLevel, arrayIndex, slice);
						memcpy(pDst, img->pixels, img->slicePitch);
						pDst += img->slicePitch;
					}
				}
			}
		}
	}


	bool LoadDDSImage(const Ptr<File> & file, const Ptr<Image> & image)
	{
		using namespace DirectX;

		TexMetadata metaData;
		ScratchImage scractchImage;
		bool bSuccess = (LoadFromDDSFile(file->Path().c_str(), DDS_FLAGS_NO_LEGACY_EXPANSION, &metaData, scractchImage) == S_OK);

		if (!bSuccess)
			return bSuccess;

		DXImageToImage(scractchImage, image);

		return true;
	}

	void SaveDDSImage(const Ptr<File> & outFile, const Ptr<Image> & image)
	{
		auto path = outFile->Path();
		outFile->Release();

		using namespace DirectX;

		std::vector<DirectX::Image> dxImages;
		TexMetadata metaData;
		ImageToDXImage(image, dxImages, metaData);

		SaveToDDSFile(&dxImages[0], dxImages.size(), metaData, DDS_FLAGS_NONE, path.c_str());
	}

	Ptr<Image> BlockCompress(const Ptr<Image> & image, RenderFormat compressFormat)
	{
		using namespace DirectX;

		if (IsCompress(image->GetFormat()) || !IsCompress(compressFormat))
			return nullptr;

		std::vector<DirectX::Image> dxImages;
		TexMetadata metaData;
		ImageToDXImage(image, dxImages, metaData);

		ScratchImage compressedScratchImage;
		Compress(&dxImages[0], dxImages.size(), metaData, static_cast<DXGI_FORMAT>(compressFormat), 0, 0.0f, compressedScratchImage);

		auto outImage = std::make_shared<Image>();
		DXImageToImage(compressedScratchImage, outImage);

		return outImage;
	}
}