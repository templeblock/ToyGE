#pragma once
#ifndef ASSERT_H
#define ASSERT_H

#include "ToyGE\Kernel\PreIncludes.h"

void TOYGE_CORE_API _assert_log(const std::string & str);

template<class CharT>
void _toyge_assert(const CharT * expr, const char * msg, const char * file, long line)
{
	std::stringstream ss;
	ss << "**** ASSERT FAIL ****" << END_SYMBOL
		<< "Expr : " << expr << END_SYMBOL
		<< "Msg  : " << msg << END_SYMBOL
		<< "File : " << file << END_SYMBOL
		<< "Line : " << line << END_SYMBOL
		<< "*********************" << END_SYMBOL;

#ifdef ASSERT_CERR
	std::cerr << ss.str();
#endif

#ifdef ASSERT_MSG_BOX
#ifdef PLATFORM_WINDOWS
	::MessageBoxA(0, ss.str().c_str(), 0, 0);
#endif
#endif

#ifdef ASSERT_LOG
	::_assert_log(ss.str());
#endif

	//std::abort();
	throw std::runtime_error("");
}

#if defined(DEBUG) || defined(_DEBUG)

#define ToyGE_ASSERT(expr) ( (!!(expr)) ? ((void)0) : ::_toyge_assert((#expr), "<no msg>",  __FILE__, __LINE__ ) )
#define ToyGE_ASSERT_MSG(expr, msg) ( (!!(expr)) ? ((void)0) : ::_toyge_assert((#expr), msg,  __FILE__, __LINE__ ) )
#define ToyGE_ASSERT_FAIL(msg) ( _toyge_assert("<fail>", msg,  __FILE__, __LINE__  ) )

#else

#define ToyGE_ASSERT(expr) (expr)
#define ToyGE_ASSERT_MSG(expr, msg) (expr)
#define ToyGE_ASSERT_FAIL(msg) ((void)0)

#endif


#endif