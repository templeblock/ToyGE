#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\MeshAsset.h"
#include "ToyGE\Kernel\MaterialAsset.h"

using namespace ToyGE;

std::string srcPath;
bool bLeftHanded = false;
bool bFlipUV = false;

void PrintHelp()
{
	printf("ModelConverter srcPath [-l] [-f] \n");
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
		if (param == "-l")
			bLeftHanded = true;
		else if (param == "-f")
			bFlipUV = true;
	}

	return true;
}

class ModelConverter : public App
{
public:
	ModelConverter()
	{

	}

	virtual void Init() override
	{
		srcPath = IdenticalPath( Global::GetPlatform()->GetPathFullName(srcPath) );
		Asset::SetAssetsBasePath(ParentPath(srcPath));

		auto meshAsset = std::make_shared<MeshAsset>();

		meshAsset->SetPath(srcPath);
		bool bSuccess = AssimpLoadMesh(meshAsset, bFlipUV, bLeftHanded);
		if (!bSuccess)
		{
			printf("Cannot convert %s\n", srcPath.c_str());
			return;
		}

		for (auto & meshElement : meshAsset->GetData())
			meshElement->material->SetDirty(true);

		meshAsset->Save();

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

	EngineDriver::Init(std::make_shared<ModelConverter>());
	EngineDriver::Run();

	return 0;
}