#pragma once
#ifndef	D3D11RENDERENGINECREATEDEF_H
#define D3D11RENDERENGINECREATEDEF_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\RenderEngine\RenderEngineCreateDef.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"

extern "C"
{
	TOYGE_D3D11RE_API void CreateRenderEngine(const std::shared_ptr<ToyGE::Window> & window, ToyGE::RenderEngine **ppRenderEngine);
}

#endif