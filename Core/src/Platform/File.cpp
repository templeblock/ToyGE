#include "ToyGE\Platform\File.h"
#include "ToyGE\Kernel\Core.h"

namespace ToyGE
{
	String IdenticalPath(const String & path)
	{
		auto identicalPath = path;
		for (auto & c : identicalPath)
		{
			if (c == '\\')
				c = '/';
		}
		auto splits = SplitString<StringEncode::UTF_8>(identicalPath, "/");
		for (int i = 0; i < (int)splits.size(); ++i)
		{
			auto j = i - 1;
			while (i < (int)splits.size() && splits[i] == "..")
			{
				while (j >= 0 && splits[j].size() == 0)
					--j;

				if (j < 0 || splits[j] == "..")
					break;
				else
				{
					splits[i] = "";
					++i;
					splits[j] = "";
					--j;
				}
			}
		}

		std::stringstream ss;
		bool bfirst = true;
		for (auto & s : splits)
		{
			if (s.size() > 0 && s != ".")
			{
				ss << (bfirst ? "" : "/") << s;
				bfirst = false;
			}
		}

		return ss.str();
	}

	String ParentPath(const String & path)
	{
		return IdenticalPath(path + "/..");
	}

	void MakePathDirectorys(const String & path)
	{
		if (!Global::GetPlatform()->FileExists(path))
		{
			Global::GetPlatform()->MakeDirectoryRecursively(path);
		}
	}

	File::File(const String & filePath, uint32_t openFlag)
		: _flag(openFlag)
	{
		_path = Global::GetPlatform()->GetPathFullName(filePath);

		_bDirectory = Global::GetPlatform()->IsPathDirectory(_path);

		auto splatFind = _path.rfind('/');
		auto dotFind = _path.rfind('.');

		int32_t splatIndex, dotIndex;

		if (splatFind == std::string::npos)
			splatIndex = -1;
		else
			splatIndex = (int)splatFind;

		if (dotFind == std::string::npos)
			dotIndex = (int)_path.size();
		else
			dotIndex = (int)dotFind;

		_name = _path.substr(splatIndex + 1, dotIndex - splatIndex);
		if (splatIndex < (int)_path.size())
			_extension = _path.substr(dotIndex);
	}
}