#include "ToyGE\Kernel\Asset.h"
#include "ToyGE\Kernel\Core.h"

namespace ToyGE
{
	String Asset::_assetsBasePath;
	std::map<String, Ptr<Asset>> Asset::_assetsMap;

	String Asset::GetLoadFullPath() const
	{
		return Global::GetPlatform()->GetPathFullName(GetLoadPath());
	}

	String Asset::GetSaveFullPath() const
	{
		return Global::GetPlatform()->GetPathFullName(GetSavePath());
	}
}