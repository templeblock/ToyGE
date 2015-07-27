#include "ToyGE\RenderEngine\Font\SignedDistanceFieldFontFactory.h"
#include "ToyGE\RenderEngine\Font\SignedDistanceFieldFont.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{
	Ptr<Font> SignedDistanceFieldFontFactory::LoadFont(const Ptr<File> & file)
	{
		//auto file = std::make_shared<File>(path, FILE_OPEN_READ);
		auto font = SignedDistanceFieldFont::Load(file);

		if (!font->IsBinLoaded())
		{
			font->Init(32, 32);
			auto sdfWritePath = file->Path().substr(0, file->Path().rfind(L".")) + L".sdff";
			font->SaveBin(std::make_shared<FileWriter>(std::make_shared<File>(sdfWritePath, FILE_OPEN_WRITE)));
		}

		return font;
	}
}