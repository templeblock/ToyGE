#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

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
}