#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\TextureAsset.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

using namespace ToyGE;

String srcPath;
float scale = 1.0f;

void PrintHelp()
{
	printf("HeightToBump srcPath [-s <scale>] [-f] \n");
}

bool ProcessCmd(int nArgs, const char ** args)
{
	if (nArgs <= 1)
	{
		printf("incorrect params\n");
		return false;
	}

	srcPath = args[1];
	if (srcPath == "?")
	{
		PrintHelp();
		return false;
	}

	for (int i = 2; i < nArgs; ++i)
	{
		std::string param = args[i];
		if (param == "-s")
		{
			if (i < nArgs - 1)
				scale = std::stof(args[i + 1]);
			else
				PrintHelp();
		}
	}

	return true;
}

class HeightToBump : public App
{
public:
	HeightToBump()
	{

	}

	virtual void Init() override
	{
		srcPath = IdenticalPath(Global::GetPlatform()->GetPathFullName(srcPath));
		Asset::SetAssetsBasePath(ParentPath(srcPath));

		auto texAsset = std::make_shared<TextureAsset>();

		texAsset->SetPath(srcPath);
		texAsset->Load();
		texAsset->Init();

		auto heightTex = texAsset->GetTexture();
		auto bumpTex = ToyGE::HeightToBumpTex(heightTex, scale);

		auto dstPath = srcPath.substr(0, srcPath.rfind('.')) + "_bump" + TextureAsset::GetExtension();

		auto bumpTexAsset = std::make_shared<TextureAsset>();
		bumpTexAsset->SetTexture(bumpTex);
		bumpTex->SetDirty(true);
		bumpTexAsset->SetPath(dstPath);
		bumpTexAsset->Save();

		Global::GetLooper()->SetExit(true);
	}

	virtual void Update(float elapsedTime) override
	{

	}

	virtual void Destroy() override
	{

	}

};

int main(int nArgs, const char ** args)
{
	if (!ProcessCmd(nArgs, args))
		return -1;

	EngineDriver::Init(std::make_shared<HeightToBump>());
	EngineDriver::Run();

	return 0;
}