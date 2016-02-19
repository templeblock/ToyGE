#pragma once
#ifndef WINDOWSPLATFORMFACTORY_H
#define WINDOWSPLATFORMFACTORY_H

#include "ToyGE\Platform\Platform.h"

namespace ToyGE
{
	class WindowsPlatform : public Platform
	{
	public:
		virtual void Init() override;

		virtual Ptr<Window> CreatePlatformWindow(const WindowCreateParams & params) override;

		virtual Ptr<Looper> CreateLooper() override;

		virtual void ShowMessage(const String & msg) override;

		
		virtual Ptr<File> CreatePlatformFile(const String & path, uint32_t openFlag) override;

		virtual String GetPathFullName(const String & path) override;

		virtual bool GetRelativePath(const String & from, bool bFromDirectory, const String & to, bool bToDirectory, String & outRelativePath) override;

		virtual bool FileExists(const String & path) override;

		virtual bool IsPathDirectory(const String & path) override;

		virtual bool MakeDirectory(const String & path) override;

		virtual String GetCurentProgramPath() override;

		virtual void SetWorkingDirectory(const String & path) override;

		virtual String GetWorkingDirectory() override;
	};
}

#endif