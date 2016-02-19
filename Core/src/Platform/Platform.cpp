#include "ToyGE\Platform\Platform.h"
#include "ToyGE\Platform\DllLoader.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Platform\Windows\WindowsDllLoader.h"
#include "ToyGE\Platform\DllObj.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Platform\File.h"

namespace ToyGE
{
	std::map<String, Ptr<DllObj>> Platform::_dllObjMap;

	Ptr<DllObj> Platform::FindDll(const String & path)
	{
		auto itr = _dllObjMap.find(path);
		if (itr != _dllObjMap.end())
		{
			return itr->second;
		}
		else
		{
			auto dll = _dllLoader->LoadDll(path);
			if (dll)
				_dllObjMap[path] = dll;
			return dll;
		}
	}

	void Platform::Clear()
	{
		_dllObjMap.clear();
	}

	bool Platform::MakeDirectoryRecursively(const String & path)
	{
		if (FileExists(path))
		{
			if (IsPathDirectory(path))
				return true;
			else
				return false;
		}
		return MakeDirectoryRecursively(ParentPath(path)) && MakeDirectory(path);
	}
}