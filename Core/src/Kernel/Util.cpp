#include "ToyGE\Kernel\Util.h"
#include "boost\functional\hash.hpp"
#include <cstdio>

namespace ToyGE
{
	uint64_t Hash(const void * src, size_t size)
	{
		return boost::hash_range(static_cast<const uint8_t*>(src), static_cast<const uint8_t*>(src) + size);
	}
}