#pragma once
#ifndef STATICCASTABLE_H
#define STATICCASTABLE_H

#include "ToyGE\Kernel\PreIncludes.h"

namespace ToyGE
{
	class TOYGE_CORE_API StaticCastable : public std::enable_shared_from_this<StaticCastable>
	{
	public:
		virtual ~StaticCastable() = default;

		template <class T>
		std::shared_ptr<T> As()
		{
			return std::static_pointer_cast<T>(shared_from_this());
		}
	};
}

#endif