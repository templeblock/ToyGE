#include "ToyGE\Kernel\Config.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Util.h"

#include "rapidxml.hpp"

namespace ToyGE
{
	Config::Config()
		: windowWidth(800),
		windowHeight(600),
		windowX(0),
		windowY(0)
	{

	}

	bool Config::Load(const WString & path, Config & outConfig)
	{
		auto file = std::make_shared<File>(path, FILE_OPEN_READ);
		if (!file->CanRead())
		{
			Logger::LogLine("error> can not open config file %s", path.c_str());
			return false;
		}

		auto buf = std::unique_ptr<uint8_t[]>(new uint8_t[file->Size() + sizeof(wchar_t)]);
		auto bytesRead = file->Read(buf.get(), file->Size());
		buf[file->Size()] = 0;
		buf[file->Size() + 1] = 0;
		if (bytesRead != file->Size())
			return false;

		rapidxml::xml_document<wchar_t> doc;
		doc.parse<0>(reinterpret_cast<wchar_t*>(buf.get()));

		auto configNode = doc.first_node(L"config");

		auto windowNode = configNode->first_node(L"window");
		if (windowNode)
		{
			auto windowTitleAttrib = windowNode->first_attribute(L"title");
			if (windowTitleAttrib)
				outConfig.windowTitle = windowTitleAttrib->value();
			auto windowWidthAttrib = windowNode->first_attribute(L"width");
			if (windowWidthAttrib)
				outConfig.windowWidth = std::stoi(windowWidthAttrib->value());
			else
				outConfig.windowWidth = 800;
			auto windowHeightAttrib = windowNode->first_attribute(L"height");
			if (windowHeightAttrib)
				outConfig.windowHeight = std::stoi(windowHeightAttrib->value());
			else
				outConfig.windowHeight = 600;
			auto windowXAttrib = windowNode->first_attribute(L"x");
			if (windowXAttrib)
				outConfig.windowX = std::stoi(windowXAttrib->value());
			else
				outConfig.windowX = 0;
			auto windowYAttrib = windowNode->first_attribute(L"y");
			if (windowYAttrib)
				outConfig.windowY = std::stoi(windowYAttrib->value());
			else
				outConfig.windowY = 0;
		}

		auto resourceNode = configNode->first_node(L"resource");
		while (resourceNode)
		{
			WString wName = resourceNode->first_attribute(L"name")->value();
			String name;
			ConvertStr_WToA(wName, name);
			WString basePath = resourceNode->first_attribute(L"path")->value();

			outConfig.resourceMap[name] = basePath;

			resourceNode = resourceNode->next_sibling(L"resource");
		}

		return true;
	}
}