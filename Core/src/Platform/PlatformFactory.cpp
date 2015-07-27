#include "ToyGE\Platform\PlatformFactory.h"
#include "ToyGE\Platform\DllLoader.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Platform\Windows\WindowsDllLoader.h"
#include "ToyGE\Platform\DllObj.h"

namespace ToyGE
{
	std::map<String, Ptr<DllObj>> PlatformFactory::_dllObjMap;

	void PlatformFactory::Clear()
	{
		_dllObjMap.clear();
	}

	PlatformFactory::PlatformFactory()
	{
#ifdef PLATFORM_WINDOWS
		_dllLoader = std::make_shared<WindowsDllLoader>();
#endif
	}

	Ptr<DllObj> PlatformFactory::AcquireDll(const String & path)
	{
		auto itr = _dllObjMap.find(path);
		if (itr != _dllObjMap.end())
		{
			return itr->second;
		}
		else
		{
			auto dll = _dllLoader->LoadDll(path);
			if (!dll)
				Logger::LogLine("error> can not load dll '%s'", path.c_str());
			else
				_dllObjMap[path] = dll;
			return dll;
		}
	}
}