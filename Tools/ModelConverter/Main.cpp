#include "ToyGE\RenderEngine\Model.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Util.h"

using namespace ToyGE;

bool ReadPath(std::istream & is, std::string & str)
{
	std::string src;
	is >> src;
	if (src[0] == '"')
	{
		while (src.back() != '"')
		{
			if (!is)
				return false;

			is >> src;
		}
	}
	return true;
}

void ProcessCmd(int nArgs, const char ** args)
{
	std::string src;
	int32_t srcArgsIndex = 0;
	for (int32_t i = 1; i < nArgs; ++i)
	{
		if (args[i][0] != '-')
		{
			src = args[i];
			srcArgsIndex = i;
		}
	}

	std::string dst;
	if (srcArgsIndex < nArgs - 1)
	{
		for (int32_t i = srcArgsIndex + 1; i < nArgs; ++i)
		{
			if (args[i][0] != '-')
				dst = args[i];
		}
	}
	else
	{
		dst = src.substr(0, src.find_first_of('.')) + ".tx";
	}


	bool bFlipUV = false;
	for (int32_t i = 0; i < nArgs; ++i)
	{
		if (args[i] == String("-f"))
			bFlipUV = true;
	}

	if (src.size() > 0)
	{
		WString path;
		ConvertStr_AToW(src, path);

		auto srcFile = std::make_shared<ToyGE::File>(path, ToyGE::FILE_OPEN_READ);
		if (!srcFile->CanRead())
		{
			std::cout << "error> could not open file \"" << src << "\"" << std::endl;
			return;
		}
		std::cout << "info> model convert start" << std::endl;

		auto model = std::make_shared<Model>();
		auto file = std::make_shared<ToyGE::File>(path, ToyGE::FILE_OPEN_READ);
		LoadModel(file, model, bFlipUV);

		if (model)
		{
			if (dst.size() == 0)
			{
				dst = src.substr(0, src.find_first_of('.')) + ".tx";
			}

			ConvertStr_AToW(dst, path);

			auto saveFile = std::make_shared<ToyGE::File>(path, ToyGE::FILE_OPEN_WRITE);
			if (!saveFile->CanWrite())
			{
				std::cout << "error> could not create file \"" << dst << "\"" << std::endl;
				return;
			}
			model->SaveBin(std::make_shared<ToyGE::FileWriter>(saveFile));
			std::cout << "info> model convert finish" << std::endl;
		}
		else
		{
			std::cout << "error> could not load model \"" << src << "\"" << std::endl;
		}
	}
	//}
}

void main(int nArgs, const char ** args)
{
	int minArgs = 1;

	if (nArgs < minArgs + 1)
		std::cout << "error> params error( [srcPath] [opt:dstPath] )" << std::endl;

	ProcessCmd(nArgs, args);


	/*std::string line;
	while (std::getline(std::cin, line))
	{
		if (line == "quit")
			break;

		ProcessLine(line);
	}*/
}