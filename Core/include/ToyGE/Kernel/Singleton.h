#pragma once
#ifndef SINGLETON_H
#define SINGLETON_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include <boost\noncopyable.hpp>

namespace ToyGE
{
	template <typename T>
	class TOYGE_CORE_API SingletonCreateFuncDeclare
	{
	public:
		typedef T*(*SingletonCreateFuncType)(void);
	};

	template <typename T, typename SingletonCreateFuncDeclare<T>::SingletonCreateFuncType func = nullptr>
	class TOYGE_CORE_API Singleton : boost::noncopyable
	{
	public:
		static const T & ConstInstance()
		{
			if (!instance)
				instance = func ? func() : new T();
			return *instance;
		}

		static T & Instance()
		{
			if (!instance)
				instance = func ? func() : new T();
			return *instance;
		}

		static void ClearInstance()
		{
			delete instance;
		}

	protected:
		static T * instance;
	};

#ifdef TOYGE_CORE_SOURCE
	template <typename T, typename SingletonCreateFuncDeclare<T>::SingletonCreateFuncType func>
	T * Singleton<T, func>::instance;
#endif
}

#endif
