#pragma once
#ifndef DLLOBJ_H
#define DLLOBJ_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "boost\noncopyable.hpp"

namespace ToyGE
{
	class TOYGE_CORE_API DllObj : public boost::noncopyable
	{
	public:
		virtual ~DllObj() = default;

		virtual void * GetProcAddress(const String & name) = 0;
	};
}

#endif