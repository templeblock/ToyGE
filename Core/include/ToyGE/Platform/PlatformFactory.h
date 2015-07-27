#pragma once
#ifndef PLATFORMFACTORY_H
#define PLATFORMFACTORY_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Platform\Window.h"

namespace ToyGE
{
	class Looper;
	class DllObj;
	class DllLoader;

	class TOYGE_CORE_API PlatformFactory
	{
	public:
		static void Clear();

		PlatformFactory();

		virtual ~PlatformFactory() = default;

		virtual Ptr<Window> CreateRenderWindow(const WindowCreateParams & params) = 0;

		virtual Ptr<Looper> CreateLooper() = 0;

		Ptr<DllObj> AcquireDll(const String & path);

		virtual void ShowMessage(const String & msg) = 0;

	private:
		Ptr<DllLoader> _dllLoader;
		static std::map<String, Ptr<DllObj>> _dllObjMap;
	};
}

#endif