#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Kernel\Assert.h"

namespace ToyGE
{
	class TOYGE_CORE_API Texture : public RenderResource
	{
		friend class RenderFactory;
	public:
		Texture(const TextureDesc & desc);

		Texture(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList);

		virtual ~Texture() = default;

		virtual bool GenerateMips() = 0;

		virtual bool CopyTo(const Ptr<Texture> & dst,
			int32_t dstMipLevel, int32_t dstArrayIndex, int32_t xOffset, int32_t yOffset, int32_t zOffset,
			int32_t srcMipLevel, int32_t srcArrayIndex, const std::shared_ptr<Box> & srcBox = std::shared_ptr<Box>()) = 0;

		virtual RenderDataDesc Map(MapType mapFlag, int32_t mipLevel, int32_t arrayIndex) = 0;

		virtual void UnMap() = 0;

		const TextureDesc & Desc() const
		{
			return _desc;
		}

		const std::tuple<int32_t, int32_t, int32_t> & GetMipSize(int32_t mipLevel) const
		{
			ToyGE_ASSERT(mipLevel < _mipSizeMap.size());
			return _mipSizeMap[mipLevel];
		}

		bool IsActive()
		{
			return _bActive;
		}

		void Release();

		static int32_t ComputeMipLevels(
			int32_t maxMipLevels,
			int32_t width,
			int32_t heigth,
			int32_t depth,
			std::vector<std::tuple<int32_t, int32_t, int32_t>> & outMipSizes);

	protected:
		TextureDesc _desc;
		std::vector<std::tuple<int32_t, int32_t, int32_t>> _mipSizeMap;
		bool _bActive;

		Texture();

		static bool CheckDesc(const TextureDesc & desc);
	};
}

#endif