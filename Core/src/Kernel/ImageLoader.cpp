#include "ToyGE\Kernel\ImageLoader.h"
#include "FreeImage.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Image.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	bool TOYGE_CORE_API LoadCommonImage(const Ptr<File> & file, const Ptr<Image> & image)
	{
		ToyGE_ASSERT(file);
		ToyGE_ASSERT(image);
		if (!file->CanRead())
			return false;

		String path;
		ConvertStr_WToA(file->Path(), path);

		auto imgFmt = FreeImage_GetFileType(path.c_str());
		if (FIF_UNKNOWN == imgFmt)
			return false;
		auto *pImg = FreeImage_Load(imgFmt, path.c_str());
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
				image->_format = RENDER_FORMAT_R8_UNORM;
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
				image->_format = RENDER_FORMAT_R8G8B8A8_UNORM;
			}

			image->_width = static_cast<int32_t>(imgInfo->bmiHeader.biWidth);
			image->_height = static_cast<int32_t>(imgInfo->bmiHeader.biHeight);
			image->_depth = static_cast<int32_t>(imgInfo->bmiHeader.biPlanes);
			image->_type = TEXTURE_2D;

			RenderDataDesc renderData;
			renderData.pData = &buffer[0];
			renderData.rowPitch = image->_width * bufferBytesPerPixel;
			renderData.slicePitch = image->_height * renderData.rowPitch;
			image->_data.push_back(renderData);
			image->_arraySize = 1;
			image->_mipLevels = 1;
			image->_rawImageData = std::shared_ptr<uint8_t>(buffer.release(), [](uint8_t *p){delete[] p; });

			return true;
		}
		else
		{
			return false;
		}

		
	}
}