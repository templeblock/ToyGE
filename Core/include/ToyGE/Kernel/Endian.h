#pragma once
#ifndef ENDIAN_H
#define ENDIAN_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include <boost\endian\arithmetic.hpp>

namespace ToyGE
{
	enum Endian
	{
		LITTLE,
		BIG,
		UNKOWN
	};

//	constexpr Endian GetEndian()
//	{
//#ifdef BOOST_LITTLE_ENDIAN
//		return Endian::LITTLE;
//#elif defined BOOST_BIG_ENDIAN
//		return Endian::BIG;
//#else
//		return Endian::UNKOWN;
//#endif
//	}

	namespace EndianDef
	{
		enum _EndianDef
		{
#ifdef BOOST_LITTLE_ENDIAN
			DEF = Endian::LITTLE
#elif defined BOOST_BIG_ENDIAN
			DEF = Endian::BIG
#else
			DEF = Endian::UNKOWN
#endif
		};
	}
}

#endif
