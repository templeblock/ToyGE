#include "ToyGE\Platform\Windows\WindowsDllLoader.h"
#include "ToyGE\Platform\Windows\WindowsDllObj.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	Ptr<DllObj> WindowsDllLoader::LoadDll(const String & name)
	{
		WString wStr;
		ConvertStr_AToW(name, wStr);

		HDLL hDll = ::LoadLibraryW( wStr.c_str() );

		return hDll ? WindowsDllObj::Create(hDll) : WindowsDllObj::Create(0);
	}
}