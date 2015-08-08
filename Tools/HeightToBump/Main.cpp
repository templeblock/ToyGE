#include "ToyGE\ToyGE.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\Kernel\ImageHelper.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include <iostream>

using namespace ToyGE;

void GetParams(
	const std::vector<String> & args,
	WString & srcPath,
	WString & dstPath,
	bool & bCompress,
	float & scale)
{
	bCompress = false;
	scale = 1.0f;

	size_t argIndex = 0;
	while (argIndex < args.size())
	{
		auto & argStr = args[argIndex];

		if (argStr[0] != '-')
		{
			if (srcPath.size() == 0)
			{
				ConvertStr_AToW(argStr, srcPath);
				++argIndex; continue;
			}
			else
			{
				ConvertStr_AToW(argStr, dstPath);
				++argIndex; continue;
			}
		}
		else
		{
			if (argStr[1] == 'c')
			{
				bCompress = true;
				++argIndex; continue;
			}
			else if (argStr[1] == 's')
			{
				scale = std::stof(args[argIndex + 1]);
				argIndex += 2; continue;
			}
		}
	}

	if (dstPath.size() == 0)
		dstPath = srcPath.substr(0, srcPath.rfind('.')) + L"_bump.dds";
}

int main(int nArgs, const char ** inArgs)
{
	//Init Params
	std::vector<String> args;
	for (int i = 1; i < nArgs; ++i)
		args.push_back(inArgs[i]);

	WString heightImagePath;
	WString bumpImagePath;
	bool bCompress;
	float scale;
	GetParams(args, heightImagePath, bumpImagePath, bCompress, scale);

	//Startup Engine
	File::SetCurrentPathToProgram();
	Config config;
	Config::Load(L"../../../Media/Config.xml", config);
	EngineDriver::StartUp(config, nullptr);

	//Load Height Image
	auto heightTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTextureFromFile(heightImagePath, TEXTURE_BIND_SHADER_RESOURCE, 0);

	//Height To Bump
	auto bumpTex = HeightToBump(heightTex, scale);

	Ptr<Image> outImage = bumpTex->CreateImage(true);

	//Compress
	if (bCompress)
		outImage = BlockCompress(outImage, RENDER_FORMAT_BC3_UNORM);

	//Save DDS File
	auto outFile = std::make_shared<File>(bumpImagePath, FILE_OPEN_WRITE);
	outFile->MakeParentPathDirectory();
	SaveDDSImage(outFile, outImage);

	return 0;
}