#include "ToyGE\Kernel\Assertion.h"
#include "ToyGE\Kernel\Logger.h"

namespace ToyGE
{
	void _toyge_assert(const char * expr, const char * msg, const char * file, long line)
	{
		std::stringstream ss;
		ss << "**** ASSERT FAIL ****" << TOYGE_LINE_END
			<< "Expr : " << expr << TOYGE_LINE_END
			<< "Msg  : " << msg << TOYGE_LINE_END
			<< "File : " << file << TOYGE_LINE_END
			<< "Line : " << line << TOYGE_LINE_END
			<< "*********************" << TOYGE_LINE_END;

		ToyGE::Logger::LogLine(ss.str());
		ToyGE::Logger::Release();

		throw std::runtime_error("");
	}

}