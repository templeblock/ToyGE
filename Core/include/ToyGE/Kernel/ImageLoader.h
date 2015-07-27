#pragma once
#ifndef DDSLOADER_H
#define DDSLOADER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class File;
	class Image;

	bool TOYGE_CORE_API LoadDDSImage(const Ptr<File> & file, const Ptr<Image> & image);
	bool TOYGE_CORE_API LoadCommonImage(const Ptr<File> & file, const Ptr<Image> & image);
}

#endif