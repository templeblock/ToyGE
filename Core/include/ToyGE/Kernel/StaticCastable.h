#pragma once
#ifndef STATICCASTABLE_H
#define STATICCASTABLE_H

#include "ToyGE\Kernel\PreInclude.h"

namespace ToyGE
{
	class TOYGE_CORE_API StaticCastable : public std::enable_shared_from_this<StaticCastable>
	{
	public:
		virtual ~StaticCastable() = default;

		template <class T>
		std::shared_ptr<T> Cast()
		{
			return std::static_pointer_cast<T>(shared_from_this());
		}

		template <class T>
		std::shared_ptr<const T> Cast() const
		{
			return std::static_pointer_cast<T>(shared_from_this());
		}

		template <class T>
		std::shared_ptr<T> DyCast()
		{
			return std::dynamic_pointer_cast<T>(shared_from_this());
		}

		template <class T>
		std::shared_ptr<const T> DyCast() const
		{
			return std::dynamic_pointer_cast<T>(shared_from_this());
		}
	};
}

#endif