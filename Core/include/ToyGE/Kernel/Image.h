#pragma once
#ifndef IMAGE_H
#define IMAGE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class File;

	class TOYGE_CORE_API Image
	{
		friend bool TOYGE_CORE_API LoadDDSImage(const Ptr<File> & file, const Ptr<Image> & image);
		friend bool TOYGE_CORE_API LoadCommonImage(const Ptr<File> & file, const Ptr<Image> & image);

	public:
		static Ptr<Image> Load(const Ptr<File> & file);

		Image();

		TextureType Type() const
		{
			return _type;
		}

		int32_t Width() const
		{
			return _width;
		}

		int32_t Height() const
		{
			return _height;
		}

		int32_t Depth() const
		{
			return _depth;
		}

		int32_t ArraySize() const
		{
			return _arraySize;
		}

		int32_t MipLevels() const
		{
			return _mipLevels;
		}

		RenderFormat Format() const
		{
			return _format;
		}

		const std::vector<RenderDataDesc> & Data() const
		{
			return _data;
		}

		bool IsDDS() const
		{
			return _isDDS;
		}

	private:
		TextureType _type;
		int32_t _width;
		int32_t _height;
		int32_t _depth;
		int32_t _arraySize;
		int32_t _mipLevels;
		RenderFormat _format;
		std::vector<RenderDataDesc> _data;
		bool _isDDS = false;

		std::shared_ptr<uint8_t> _rawImageData;
	};
}

#endif