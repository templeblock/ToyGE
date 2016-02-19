#pragma once
#ifndef PLATFORM_H
#define PLATFORM_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\Platform\File.h"
#include "ToyGE\Platform\DllObj.h"
#include "ToyGE\Platform\DllLoader.h"
#include "ToyGE\Platform\Looper.h"

namespace ToyGE
{
	class TOYGE_CORE_API Platform
	{
	public:
		virtual ~Platform() = default;

		virtual void Init() = 0;

		virtual Ptr<Window> CreatePlatformWindow(const WindowCreateParams & params) = 0;

		virtual Ptr<Looper> CreateLooper() = 0;

		virtual void ShowMessage(const String & msg) = 0;

		Ptr<DllObj> FindDll(const String & path);


		virtual Ptr<File> CreatePlatformFile(const String & path, uint32_t openFlag) = 0;

		virtual String GetPathFullName(const String & path) = 0;

		virtual bool GetRelativePath(const String & from, bool bFromDirectory, const String & to, bool bToDirectory, String & outRelativePath) = 0;

		virtual bool FileExists(const String & path) = 0;

		virtual bool IsPathDirectory(const String & path) = 0;

		virtual bool MakeDirectory(const String & path) = 0;

		virtual bool MakeDirectoryRecursively(const String & path);

		virtual String GetCurentProgramPath() = 0;

		virtual void SetWorkingDirectory(const String & path) = 0;

		virtual String GetWorkingDirectory() = 0;


		static void Clear();

	protected:
		Ptr<DllLoader> _dllLoader;
		static std::map<String, Ptr<DllObj>> _dllObjMap;
	};
}

#endif
