#include "ToyGE\RenderEngine\Font\BitmapFontFactory.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\RenderEngine\Font\BitmapFont.h"

namespace ToyGE
{
	Ptr<Font> BitmapFontFactory::LoadFont(const Ptr<File> & file)
	{
		//auto file = std::make_shared<File>(path, FILE_OPEN_READ);
		auto font = BitmapFont::Load(file);
		font->Init(32, 32);

		return font;
	}
}