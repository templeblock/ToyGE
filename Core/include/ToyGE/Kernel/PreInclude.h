#pragma once
#ifndef PREINCLUDE_H
#define PREINCLUDE_H

#pragma warning(disable: 4996)
#pragma warning(disable: 4250)
#pragma warning(disable: 4251)
#pragma warning(disable: 4503)

#if defined(WIN32) || defined(_WINDOWS) || defined(WINDOWS)
#define TOYGE_PLATFORM_WINDOWS 
#endif

#include <cstdlib>
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
#include <codecvt>

#ifdef TOYGE_PLATFORM_WINDOWS
#include <Windows.h>
#include <Windowsx.h>
#include <initguid.h>
#include <Shlwapi.h>

#define TOYGE_LINE_END "\r\n"

#endif


namespace ToyGE
{
	using String	= std::string;

	template <typename T>
	using Ptr = std::shared_ptr < T > ;

#ifdef TOYGE_PLATFORM_WINDOWS
	using HDLL = HMODULE;
	using HWINDOW = HWND;
#endif
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
	}

#endif