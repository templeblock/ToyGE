#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Platform\File.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Platform\Platform.h"

namespace ToyGE
{
	static Ptr<FileWriter> _logWriter;

	void Logger::Init()
	{
		auto logPath = ParentPath(Global::GetPlatform()->GetCurentProgramPath()) + "/ToyGE.log";
		_logWriter = std::make_shared<FileWriter>(Global::GetPlatform()->CreatePlatformFile(logPath, FILE_OPEN_WRITE));
	}

	void Logger::Release()
	{
		_logWriter = nullptr;
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
		_logWriter->WriteStringNoTerminates(FormatString(format, vl));
		_logWriter->GetFile()->Flush();
	}

	void Logger::Log(const String & msg)
	{
		Log(msg.c_str());
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
		_logWriter->WriteStringNoTerminates(FormatString(format, vl));
		_logWriter->WriteStringNoTerminates(TOYGE_LINE_END);
		_logWriter->GetFile()->Flush();
	}

	void Logger::LogLine(const String & msg)
	{
		LogLine(msg.c_str());
	}


	void ToyGELog(LogType type, const char * format, ...)
	{
		va_list vl;
		va_start(vl, format);
		ToyGELog(type, format, vl);
		va_end(vl);
	}

	void ToyGELog(LogType type, const char * format, va_list vl)
	{
		String prefix;
		switch (type)
		{
		case LogType::LT_INFO:
			prefix = "Info> ";
			break;
		case LogType::LT_WARNING:
			prefix = "Warning> ";
			break;
		case LogType::LT_ERROR:
			prefix = "Error> ";
			break;
		default:
			break;
		}

		Logger::LogLine((prefix + format).c_str(), vl);
	}

	void ToyGELog(LogType type, const String & msg)
	{
		ToyGELog(type, msg.c_str());
	}
}



