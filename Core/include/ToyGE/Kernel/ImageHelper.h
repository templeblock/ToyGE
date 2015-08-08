#pragma once
#ifndef IMAGEHELPER_H
#define IMAGEHELPER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class File;
	class Image;

	TOYGE_CORE_API bool LoadCommonImage(const Ptr<File> & file, const Ptr<Image> & outImage);

	TOYGE_CORE_API bool LoadDDSImage(const Ptr<File> & file, const Ptr<Image> & outImage);

	TOYGE_CORE_API void SaveDDSImage(const Ptr<File> & outFile, const Ptr<Image> & image);

	TOYGE_CORE_API Ptr<Image> BlockCompress(const Ptr<Image> & image, RenderFormat compressFormat);
}

#endif