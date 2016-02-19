#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class TOYGE_CORE_API Logger
	{
	public:
		static void Init();

		static void Release();

		static void Log(const char * format, ...);

		static void Log(const char * format, va_list vl);

		static void Log(const String & msg);

		static void LogLine(const char * format, ...);

		static void LogLine(const char * format, va_list vl);

		static void LogLine(const String & msg);
	};

	enum LogType
	{
		LT_INFO,
		LT_WARNING,
		LT_ERROR,
		LT_RAW
	};

	TOYGE_CORE_API void ToyGELog(LogType type, const char * format, ...);
	TOYGE_CORE_API void ToyGELog(LogType type, const char * format, va_list vl);
	TOYGE_CORE_API void ToyGELog(LogType type, const String & msg);

#define ToyGE_LOG ToyGELog
}

#endif