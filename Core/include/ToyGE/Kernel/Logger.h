#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Kernel\File.h"

namespace ToyGE
{
	class TOYGE_CORE_API Logger
	{
	public:
		static void Init();

		static void Log(const char * format, ...);

		static void Log(const char * format, va_list vl);

		static void LogLine(const char * format, ...);

		static void LogLine(const char * format, va_list vl);

	private:
		static Ptr<Writer> _logWriter;
	};
}

#endif