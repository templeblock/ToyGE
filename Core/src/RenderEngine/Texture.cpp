#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Kernel\Image.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	Texture::Texture(const TextureDesc & desc)
		: Texture(desc, std::vector<RenderDataDesc>())
	{

	}

	Texture::Texture(const TextureDesc & desc, const std::vector<RenderDataDesc> & initData)
		: RenderResource(RENDER_RESOURCE_TEXTURE),
		_bActive(true)
	{
		if (CheckDesc(desc))
		{
			_desc = desc;
			//if (_desc.mipLevels == 0)
			_desc.mipLevels = ComputeMipLevels(_desc.mipLevels, _desc.width, _desc.height, _desc.depth, _mipSizeMap);
		}
	}

	bool Texture::CheckDesc(const TextureDesc & desc)
	{
		if (desc.type == TEXTURE_UNDEFINED
			|| desc.format == RENDER_FORMAT_UNDEFINED)
			return false;

		if (desc.type == TEXTURE_1D && desc.width == 0)
			return false;
		if (desc.type == TEXTURE_2D && (desc.width == 0 || desc.height == 0))
			return false;
		if (desc.type == TEXTURE_3D && (desc.width == 0 || desc.height == 0 || desc.depth == 0))
			return false;

		if (desc.arraySize == 0)
			return false;
		if (desc.sampleCount == 0)
			return false;

		return true;
	}

	void Texture::Release()
	{
		if (_bActive)
			Global::GetRenderEngine()->GetRenderFactory()->ReleaseTextureToPool(std::static_pointer_cast<Texture>(shared_from_this()));
	}

	Texture::Texture()
		: RenderResource(RENDER_RESOURCE_TEXTURE)
	{

	}

	int32_t Texture::ComputeMipLevels(
		int32_t maxMipLevels,
		int32_t width,
		int32_t heigth,
		int32_t depth,
		std::vector<std::tuple<int32_t, int32_t, int32_t>> & outMipSizes)
	{
		int32_t x = std::max<int32_t>(1, width);
		int32_t y = std::max<int32_t>(1, heigth);
		int32_t z = std::max<int32_t>(1, depth);

		outMipSizes.push_back(std::make_tuple(x, y, z));
		int32_t mipLevels = 1;

		while ((x != 1) || (y != 1) || (z != 1))
		{
			++mipLevels;
			x = std::max<int32_t>(1, x >> 1);
			y = std::max<int32_t>(1, y >> 1);
			z = std::max<int32_t>(1, z >> 1);

			if (maxMipLevels > 0)
			{
				if (mipLevels <= maxMipLevels)
					outMipSizes.push_back(std::make_tuple(x, y, z));
				else
					return mipLevels - 1;
			}
			else
				outMipSizes.push_back(std::make_tuple(x, y, z));
		} 

		return mipLevels;
	}

	Ptr<Image> Texture::CreateImage(bool bWithMipMap) const
	{
		auto image = std::make_shared<Image>();

		image->SetType(Desc().type);
		image->SetWidth(Desc().width);
		image->SetHeight (Desc().height);
		image->SetDepth(Desc().depth);
		image->SetArraySize(Desc().arraySize);
		image->SetMipLevels(bWithMipMap ? Desc().mipLevels : 1);
		image->SetFormat(Desc().format);

		int32_t adjustArraySize = image->GetArraySize();
		if (image->GetType() == TEXTURE_CUBE)
			adjustArraySize *= 6;

		auto dumpTexDesc = Desc();
		dumpTexDesc.mipLevels = image->GetMipLevels();
		dumpTexDesc.bindFlag = 0;
		dumpTexDesc.cpuAccess = CPU_ACCESS_READ;
		auto dumpTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(dumpTexDesc);

		for (int32_t arrayIndex = 0; arrayIndex < adjustArraySize; ++arrayIndex)
		{
			if (bWithMipMap)
			{
				for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
				{
					this->CopyTo(dumpTex, mipLevel, arrayIndex, 0, 0, 0, mipLevel, arrayIndex);
				}
			}
			else
			{
				this->CopyTo(dumpTex, 0, arrayIndex, 0, 0, 0, 0, arrayIndex);
			}
		}

		int32_t pixelBits = GetRenderFormatNumBits(dumpTexDesc.format);
		bool bCompression = IsCompress(dumpTexDesc.format);

		int32_t dataSize = 0;
		for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
		{
			if (!(mipLevel == 0 || bWithMipMap))
				break;

			auto mipSize = dumpTex->GetMipSize(mipLevel);
			int32_t w = std::get<0>(mipSize);
			int32_t h = std::get<1>(mipSize);
			int32_t d = std::get<2>(mipSize);

			if (bCompression)
			{
				w = (w + 3) / 4 * 4;
				h = (h + 3) / 4 * 4;

				auto pitch = (16 * pixelBits / 8) * w / 4;
				auto slice = pitch * h / 4;
				dataSize += slice * d;
			}
			else
			{
				auto pitch = pixelBits / 8 * w;
				auto slice = pitch * h;
				dataSize += slice * d;
			}
		}
		dataSize *= adjustArraySize;
		image->Data() = MakeBufferedDataShared(dataSize);

		uint8_t * pDst = static_cast<uint8_t*>(image->Data().get());

		for (int32_t arrayIndex = 0; arrayIndex < adjustArraySize; ++arrayIndex)
		{
			for (int32_t mipLevel = 0; mipLevel < dumpTexDesc.mipLevels; ++mipLevel)
			{
				auto mipSize = dumpTex->GetMipSize(mipLevel);
				int32_t w = std::get<0>(mipSize);
				int32_t h = std::get<1>(mipSize);
				int32_t d = std::get<2>(mipSize);

				int32_t cpyPitch = 0;
				int32_t numCpys = 0;

				if (bCompression)
				{
					w = (w + 3) / 4 * 4;
					h = (h + 3) / 4 * 4;

					cpyPitch = (16 * pixelBits / 8) * (w / 4);
					numCpys = h / 4;
				}
				else
				{
					cpyPitch = pixelBits / 8 * w;
					numCpys = h;
				}

				auto mappedData = dumpTex->Map(MAP_READ, mipLevel, arrayIndex);

				RenderDataDesc dataDesc;
				dataDesc.pData = pDst;
				dataDesc.rowPitch = cpyPitch;
				dataDesc.slicePitch = dataDesc.rowPitch * numCpys;
				image->DataDescs().push_back(dataDesc);

				for (int32_t slice = 0; slice < d; ++slice)
				{
					uint8_t * pSrc = static_cast<uint8_t*>(mappedData.pData) + slice * mappedData.slicePitch;
					for (int32_t row = 0; row < numCpys; ++row)
					{
						memcpy(pDst, pSrc, cpyPitch);
						pSrc += mappedData.rowPitch;
						pDst += cpyPitch;
					}
				}

				dumpTex->UnMap();
			}
		}

		return image;
	}

	Ptr<Texture> Texture::CreateMips() const
	{
		auto mipsTexDesc = Desc();
		mipsTexDesc.mipLevels = 0;
		mipsTexDesc.bindFlag |= TEXTURE_BIND_GENERATE_MIPS;
		mipsTexDesc.bindFlag &= ~TEXTURE_BIND_IMMUTABLE;
		auto mipsTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(mipsTexDesc);

		int32_t adjustArraySize = Desc().arraySize;
		if (Desc().type == TEXTURE_CUBE)
			adjustArraySize *= 6;

		for (uint32_t arrayIndex = 0; arrayIndex != adjustArraySize; ++arrayIndex)
		{
			this->CopyTo(
				mipsTex, //dst
				0, //dstMipLevel
				arrayIndex, //dstArrayIndex
				0, //xOffset
				0, //yOffset
				0, //zOffset
				0, //srcMipLevel
				arrayIndex //srcArrayIndex
				);
		}

		mipsTex->GenerateMips();

		return mipsTex;
	}

}