#include "ToyGE\Platform\Windows\WindowsDllObj.h"

namespace ToyGE
{
	Ptr<DllObj> WindowsDllObj::Create(HDLL dll)
	{
		return std::shared_ptr<WindowsDllObj>(new WindowsDllObj(dll));
	}

	WindowsDllObj::WindowsDllObj(HDLL dll)
		: _hDll(dll)
	{

	}

	WindowsDllObj::~WindowsDllObj()
	{
		::FreeLibrary(_hDll);
	}

	void * WindowsDllObj::GetProcAddress(const String & name)
	{
		return ::GetProcAddress(_hDll, name.c_str());
	}
}