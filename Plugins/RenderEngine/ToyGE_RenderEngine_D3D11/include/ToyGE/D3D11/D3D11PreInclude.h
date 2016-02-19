#pragma once
#ifndef D3D11REPREDECLARE_H
#define D3D11REPREDECLARE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include <d3d11_1.h>
#include <d3dcompiler.h>

#ifdef TOYGE_D3D11RE_SOURCE
#define TOYGE_D3D11RE_API _declspec(dllexport)
#else
#define TOYGE_D3D11RE_API _declspec(dllimport)
#endif

namespace ToyGE
{
	using D3DCompileFunc = decltype(::D3DCompile) *;
	using D3DReflectFunc = decltype(::D3DReflect) *;
}

#endif