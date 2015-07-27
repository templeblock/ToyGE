#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\Assert.h"

namespace ToyGE
{
	Ptr<Writer> Logger::_logWriter;

	void Logger::Init()
	{
		_logWriter = std::make_shared<FileWriter>(std::make_shared<File>(L"../../../Media/ToyGE.log", FILE_OPEN_WRITE));
	}

	void Logger::Log(const char * format, ...)
	{
		va_list vl;
		va_start(vl, format);
		Log(format, vl);
		va_end(vl);
	}

	void Logger::Log(const char * format, va_list vl)
	{
		String outStr;
		FormatString(outStr, format, vl);
		_logWriter->WriteStringNoTerminates(outStr);
		//_logWriter->Flush();
	}

	void Logger::LogLine(const char * format, ...)
	{
		va_list vl;
		va_start(vl, format);
		LogLine(format, vl);
		va_end(vl);
	}

	void Logger::LogLine(const char * format, va_list vl)
	{
		Log(format, vl);
		_logWriter->WriteStringNoTerminates(END_SYMBOL);
	}
}

void _assert_log(const std::string & str)
{
	ToyGE::Logger::Log(str.c_str());
}