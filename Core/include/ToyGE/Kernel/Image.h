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
	public:
		static Ptr<Image> Load(const Ptr<File> & file);

		Image();

		CLASS_SET(Type, TextureType, _type);
		CLASS_GET(Type, TextureType, _type);

		CLASS_SET(Format, RenderFormat, _format);
		CLASS_GET(Format, RenderFormat, _format);

		CLASS_SET(Width, int32_t, _width);
		CLASS_GET(Width, int32_t, _width);

		CLASS_SET(Height, int32_t, _height);
		CLASS_GET(Height, int32_t, _height);

		CLASS_SET(Depth, int32_t, _depth);
		CLASS_GET(Depth, int32_t, _depth);

		CLASS_SET(ArraySize, int32_t, _arraySize);
		CLASS_GET(ArraySize, int32_t, _arraySize);

		CLASS_SET(MipLevels, int32_t, _mipLevels);
		CLASS_GET(MipLevels, int32_t, _mipLevels);

		/*TextureType Type() const
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
		}*/

		std::vector<RenderDataDesc> & DataDescs()
		{
			return _dataDescs;
		}

		const std::vector<RenderDataDesc> & DataDescs() const
		{
			return _dataDescs;
		}

		std::shared_ptr<uint8_t> & Data()
		{
			return _data;
		}

		const std::shared_ptr<uint8_t> & Data() const
		{
			return _data;
		}

	private:
		TextureType _type;
		int32_t _width;
		int32_t _height;
		int32_t _depth;
		int32_t _arraySize;
		int32_t _mipLevels;
		RenderFormat _format;
		std::vector<RenderDataDesc> _dataDescs;
		std::shared_ptr<uint8_t> _data;
	};
}

#endif