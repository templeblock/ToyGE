#include "ToyGE\Kernel\Image.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\ImageLoader.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Assert.h"

#include "FreeImage.h"

namespace ToyGE
{
	Image::Image()
	{

	}

	Ptr<Image> Image::Load(const Ptr<File> & file)
	{
		ToyGE_ASSERT(file);

		if (!file->Valid())
		{
			Logger::LogLine("error> can not open file '%s'", file->Path().c_str());
			return Ptr<Image>();
		}

		Ptr<Image> image = std::make_shared<Image>();

		WString path = file->Path();
		WString fileExtend = path.substr(path.rfind(L'.') + 1);
		std::transform(fileExtend.begin(), fileExtend.end(), fileExtend.begin(), ::tolower);
		if (fileExtend == L"dds")
		{
			if (LoadDDSImage(file, image))
			{
				image->_isDDS = true;
				return image;
			}
			else
			{
				Logger::LogLine("error> can not load image '%s'", file->Path().c_str());
				return Ptr<Image>();
			}
		}
		else
		{
			if (LoadCommonImage(file, image))
			{
				return image;
			}
			else
			{
				Logger::LogLine("error> can not load image '%s'", file->Path().c_str());
				return Ptr<Image>();
			}
		}
	}
}