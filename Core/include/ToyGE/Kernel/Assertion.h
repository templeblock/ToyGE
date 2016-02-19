#pragma once
#ifndef ASSERT_H
#define ASSERT_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	TOYGE_CORE_API void _toyge_assert(const char * expr, const char * msg, const char * file, long line);

#define ToyGE_ASSERT(expr) ( (!!(expr)) ? ((void)0) : _toyge_assert((#expr), "<no msg>",  __FILE__, __LINE__ ) )
#define ToyGE_ASSERT_MSG(expr, msg) ( (!!(expr)) ? ((void)0) : _toyge_assert((#expr), msg,  __FILE__, __LINE__ ) )
#define ToyGE_ASSERT_FAIL(msg) ( _toyge_assert("<fail>", msg,  __FILE__, __LINE__  ) )
}


#endif