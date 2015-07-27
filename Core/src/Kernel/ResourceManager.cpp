#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\Kernel\Image.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Model.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\BufferedIO.h"
#include "ToyGE\RenderEngine\Font\FontFactory.h"

namespace ToyGE
{
	Ptr<RenderEffect> EffectManager::DoLoadResource(const WString & key)
	{
		Ptr<RenderEffect> effect;

		WString rawFXPath = File::Search({ _basePath, L".\\" }, key);
		auto rawFXFile = std::make_shared<File>(rawFXPath, FILE_OPEN_READ);

		effect = RenderEffect::Load(rawFXFile);

		return effect;
	}

	void EffectManager::Clear()
	{
		for (auto & effect : _resMap)
			effect.second->Save();
		ResourceManagerDefaultMap::Clear();
	}


	Ptr<Texture> TextureManager::DoLoadResource(const WString & key)
	{
		auto path = File::Search({ _basePath, L".\\" }, key);
		Ptr<Image> img = Image::Load(std::make_shared<File>(path, FILE_OPEN_READ));
		Ptr<Texture> texture;
		auto factory = Global::GetRenderEngine()->GetRenderFactory();
		if (img->IsDDS())
			texture = factory->CreateTexture(img);
		else
			texture = factory->CreateTextureAutoGenMips(img);
		return texture;
	}


	Ptr<Model> ModelManager::DoLoadResource(const WString & key)
	{
		Ptr<Model> model;
		auto path = File::Search({ _basePath, L".\\" }, key);
		if (path.substr(path.size() - 2) == L"tx")
		{
			auto modelFile = std::make_shared<File>(_basePath + key, FILE_OPEN_READ);
			auto fileSize = modelFile->Size();
			auto buffer = MakeBufferedDataShared(fileSize);
			modelFile->Read(buffer.get(), fileSize);
			model = Model::LoadBin(std::make_shared<BufferedReader>(buffer, static_cast<uint32_t>(fileSize)), modelFile->Path());
		}
		else
		{
			model = Model::Load(std::make_shared<File>(path, FILE_OPEN_READ));
		}
		if (model)
		{
			model->RegisterResources();
			model->InitRenderData();
		}
		return model;
	}

	Ptr<Font> FontManager::DoLoadResource(const WString & key)
	{
		Ptr<Model> model;
		auto path = File::Search({ _basePath, L".\\" }, key);

		auto file = std::make_shared<File>(path, FILE_OPEN_READ);

		auto font = Global::GetRenderEngine()->GetFontFactory()->LoadFont(file);

		return font;
	}
}