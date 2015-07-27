#pragma once
#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class File;
	class Model;

	TOYGE_CORE_API bool LoadModel(const Ptr<File> & file, const Ptr<Model> & model, bool bFlipUV);
}

#endif