#include "ToyGE\Kernel\Config.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Platform\Platform.h"
#include "ToyGE\Platform\File.h"

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

	Ptr<Config> Config::Load(const String & path)
	{
		auto file = Global::GetPlatform()->CreatePlatformFile(path, FILE_OPEN_READ);
		if (!file->IsValid())
			return nullptr;

		auto config = std::make_shared<Config>();

		auto fileSize = file->Size();
		auto buf = std::unique_ptr<uint8_t[]>(new uint8_t[fileSize + 1]);
		auto bytesRead = file->Read(buf.get(), fileSize);
		buf[file->Size()] = 0;
		if (bytesRead != fileSize)
			return nullptr;

		rapidxml::xml_document<> doc;
		doc.parse<0>( reinterpret_cast<char*>(buf.get()) );

		auto configNode = doc.first_node("config");

		{
			auto node = configNode->first_node("window");
			if (node)
			{
				{
					auto attrib = node->first_attribute("title");
					if (attrib)
						config->windowTitle = attrib->value();
				}
				{
					auto attrib = node->first_attribute("width");
					if (attrib)
						config->windowWidth = std::stoi(attrib->value());
				}
				{
					auto attrib = node->first_attribute("height");
					if (attrib)
						config->windowHeight = std::stoi(attrib->value());
				}
				{
					auto attrib = node->first_attribute("x");
					if (attrib)
						config->windowX = std::stoi(attrib->value());
				}
				{
					auto attrib = node->first_attribute("y");
					if (attrib)
						config->windowY = std::stoi(attrib->value());
				}
			}
		}

		{
			auto node = configNode->first_node("adapter");
			if (node)
			{
				{
					auto attrib = node->first_attribute("index");
					if (attrib)
						config->adapterIndex = std::stoi(attrib->value());
				}
				{
					auto attrib = node->first_attribute("key");
					if (attrib)
						config->adapterSelectKey = attrib->value();
				}
			}
		}

		{
			auto node = configNode->first_node("graphics_engine");
			if (node)
			{
				{
					auto attrib = node->first_attribute("debug");
					if (attrib)
						config->bGraphicsEngineDebug = !!std::stoi(attrib->value());
				}
			}
		}

		return config;
	}
}