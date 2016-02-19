#include "ToyGE\Platform\Windows\WindowsDllLoader.h"
#include "ToyGE\Platform\Windows\WindowsDllObj.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	Ptr<DllObj> WindowsDllLoader::LoadDll(const String & name)
	{
		HDLL hDll = ::LoadLibraryW( reinterpret_cast<const wchar_t*>(StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(name).c_str()) );

		return hDll ? WindowsDllObj::Create(hDll) : WindowsDllObj::Create(0);
	}
}