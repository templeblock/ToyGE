#pragma once
#ifndef RENDERENGINECREATEDEF_H
#define RENDERENGINECREATEDEF_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\RenderEngine\RenderEngine.h"

#define CREATE_RE_FUNC_NAME "CreateRenderEngine"

namespace ToyGE
{
	using CreateRenderEngineFunc = void(*)(const std::shared_ptr<ToyGE::Window> & window, RenderEngine **ppRenderEngine);
}

#endif