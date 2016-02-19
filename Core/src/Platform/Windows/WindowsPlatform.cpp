#include "ToyGE\Platform\Windows\WindowsPlatform.h"
#include "ToyGE\Platform\Windows\WindowsWindow.h"
#include "ToyGE\Platform\Windows\WindowsLooper.h"
#include "ToyGE\Platform\Windows\WindowsDllLoader.h"
#include "ToyGE\Platform\Windows\WindowsFile.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	void WindowsPlatform::Init()
	{
		_dllLoader = std::make_shared<WindowsDllLoader>();
	}

	Ptr<Window> WindowsPlatform::CreatePlatformWindow(const WindowCreateParams & params)
	{
		auto window = std::make_shared<WindowsWindow>(params);
		if (window)
			return window;
		else
			return Ptr<Window>();
	}

	Ptr<Looper> WindowsPlatform::CreateLooper()
	{
		return std::make_shared<WindowsLooper>();
	}

	void WindowsPlatform::ShowMessage(const String & msg)
	{
		::MessageBoxA(0, msg.c_str(), 0, 0);
	}

	Ptr<File> WindowsPlatform::CreatePlatformFile(const String & path, uint32_t openFlag)
	{
		return std::make_shared<WindowsFile>(path, openFlag);
	}

	String WindowsPlatform::GetPathFullName(const String & path)
	{
		auto u16Path = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(path);

		int32_t bufLen = ::GetFullPathNameW(reinterpret_cast<const wchar_t*>(u16Path.c_str()), 0, 0, 0);
		std::basic_string<wchar_t> fullPath(bufLen, 0);
		if (::GetFullPathNameW(reinterpret_cast<const wchar_t*>(u16Path.c_str()), bufLen, &fullPath[0], 0) != bufLen - 1)
			return String();
		else
		{
			fullPath.pop_back();
			return IdenticalPath( StringConvert<StringEncode::UTF_16, StringEncode::UTF_8>(std::basic_string<char16_t>((const char16_t*)fullPath.c_str())) );
		}
	}

	bool WindowsPlatform::GetRelativePath(const String & from, bool bFromDirectory, const String & to, bool bToDirectory, String & outRelativePath)
	{
		auto fromPath = from;
		for (auto & c : fromPath)
			if (c == '/')
				c = '\\';
		auto toPath = to;
		for (auto & c : toPath)
			if (c == '/')
				c = '\\';

		auto u16FromPath = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(fromPath);
		auto u16ToPath = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(toPath);

		auto fromAttrib = bFromDirectory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
		auto toAttrib = bToDirectory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

		wchar_t relativePath[MAX_PATH];

		auto bSuccess = ::PathRelativePathToW(
			relativePath,
			reinterpret_cast<const wchar_t*>(u16FromPath.c_str()), fromAttrib,
			reinterpret_cast<const wchar_t*>(u16ToPath.c_str()), toAttrib);
		if (bSuccess)
			outRelativePath = IdenticalPath( StringConvert<StringEncode::UTF_16, StringEncode::UTF_8>(reinterpret_cast<const char16_t*>(relativePath)) );

		return !!bSuccess;
	}

	bool WindowsPlatform::FileExists(const String & path)
	{
		auto u16Path = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(path);
		return ::PathFileExistsW(reinterpret_cast<const wchar_t*>(u16Path.c_str())) == TRUE;
	}

	bool WindowsPlatform::IsPathDirectory(const String & path)
	{
		auto u16Path = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(path);
		return ::PathIsDirectoryW(reinterpret_cast<const wchar_t*>(u16Path.c_str())) == FILE_ATTRIBUTE_DIRECTORY;
	}

	bool WindowsPlatform::MakeDirectory(const String & path)
	{
		auto u16Path = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(path);
		if (::CreateDirectoryW(reinterpret_cast<const wchar_t*>(u16Path.c_str()), nullptr) == FALSE)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				return true;
			else
				return false;
		}
		return true;
	}

	String WindowsPlatform::GetCurentProgramPath()
	{
		std::vector<wchar_t> programPath(MAX_PATH);
		while (true)
		{
			::GetModuleFileNameW(nullptr, &programPath[0], static_cast<DWORD>(programPath.size()));
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				programPath.resize(programPath.size() * 2);
			else
				break;
		}

		return IdenticalPath( StringConvert<StringEncode::UTF_16, StringEncode::UTF_8>(std::basic_string<char16_t>((char16_t*)&programPath[0])) );
	}

	void WindowsPlatform::SetWorkingDirectory(const String & path)
	{
		auto u16Path = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(path);
		::SetCurrentDirectoryW(reinterpret_cast<const wchar_t*>(u16Path.c_str()));
	}

	String WindowsPlatform::GetWorkingDirectory()
	{
		int32_t bufLen = ::GetCurrentDirectoryW(0, 0);
		std::basic_string<wchar_t> path(bufLen, 0);
		if (::GetCurrentDirectoryW(bufLen, &path[0]) == 0)
			return String();
		else
		{
			path.pop_back();
			return IdenticalPath( StringConvert<StringEncode::UTF_16, StringEncode::UTF_8>(std::basic_string<char16_t>((const char16_t*)path.c_str())) );
		}
	}
}