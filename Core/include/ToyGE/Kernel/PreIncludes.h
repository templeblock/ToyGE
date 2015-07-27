#pragma once
#ifndef PREINCLUDES_H
#define PREINCLUDES_H

#include "ToyGE\Kernel\CompileConfig.h"

//#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
//#include <crtdbg.h>

#include <cassert>
#include <cstdint>
#include <cmath>
#include <memory>
#include <functional>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <stack>
#include <list>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#include <Windowsx.h>
#include <initguid.h>

using HDLL		= HMODULE;
using HWINDOW	= HWND;
#endif

#ifdef PLATFORM_WINDOWS
#define END_SYMBOL ("\r\n")
#endif

#pragma warning(disable : 4251)

namespace ToyGE
{
	using String	= std::string;
	using WString	= std::wstring;

	template <typename T>
	using Ptr = std::shared_ptr < T > ;
	template <typename T>
	using CPtr = std::shared_ptr < const T >;
}

#define CLASS_GET(Name, ParamType, Param) \
	const ParamType & Get##Name() const\
	{\
		return (Param);\
	}

#define CLASS_SET(Name, ParamType, Param) \
	void Set##Name(const ParamType & v)\
	{\
		(Param) = v;\
	}\

#endif