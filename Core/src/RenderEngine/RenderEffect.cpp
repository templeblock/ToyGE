#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderTechnique.h"
#include "ToyGE\RenderEngine\RenderEffectVariable.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderPass.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"
#include "ToyGE\Kernel\ResourceManager.h"

#include "rapidxml.hpp"

namespace ToyGE
{
	static void AddInclude(rapidxml::xml_node<> * dst, rapidxml::xml_node<> * insertPos, rapidxml::xml_document<> * include)
	{
		if (nullptr == dst || nullptr == include)
			return;

		auto effectNode = include->first_node("effect");
		if (nullptr == effectNode)
			return;

		auto subNode = effectNode->first_node();
		rapidxml::xml_node<> * next = nullptr;
		while (subNode)
		{
			next = subNode->next_sibling();
			effectNode->remove_node(subNode);
			dst->insert_node(insertPos, subNode);
			subNode = next;
		}
	}

	static void AppendNodeStr(const rapidxml::xml_node<> * node, String & outStr)
	{
		if (node->name_size() != 0)
		{
			outStr += "<";
			outStr += node->name();
			outStr += " ";
		}

		auto attrib = node->first_attribute();
		while (attrib)
		{
			outStr += attrib->name();
			outStr += "=\"";
			outStr += attrib->value();
			outStr += "\" ";

			attrib = attrib->next_attribute();
		}

		if (node->name_size() != 0)
		{
			outStr += ">";
		}

		if (node->value_size() != 0)
		{
			outStr += "<![CDATA[";
			outStr += node->value();
			outStr += "]]>";
		}

		auto subNode = node->first_node();
		while (subNode)
		{
			AppendNodeStr(subNode, outStr);
			subNode = subNode->next_sibling();
		}

		if (node->name_size() != 0)
		{
			outStr += "</";
			outStr += node->name();
			outStr += ">";
		}
	}

	static void GetXMLString(const rapidxml::xml_node<> * xml, String & outStr)
	{
		outStr += "<?xml version=\"1.0\" encoding=\"utf - 8\"?>";
		AppendNodeStr(xml, outStr);
	}

	static void SortMacros(std::vector<MacroDesc> & macros)
	{
		std::sort(macros.begin(), macros.end(), 
			[](const MacroDesc & m0, const MacroDesc & m1)->bool
		{
			return m0.name < m1.name;
		});
	}

	static FileTime GetEffectsLastedTime(const WString & basePath, const WString & effectFileName)
	{
		auto fxFile = std::make_shared<File>(basePath + effectFileName, FILE_OPEN_READ);
		auto fileTime = fxFile->GetTime();
		auto fileSize = fxFile->Size();
		//auto fxData = std::unique_ptr<uint8_t[]>(new uint8_t[fileSize]);
		auto fxData = MakeBufferedData(fileSize + 1);
		fxFile->Read(&fxData[0], fileSize);
		fxData[fileSize] = 0;

		using namespace rapidxml;
		xml_document<> doc;
		doc.parse<0>(reinterpret_cast<char*>(&fxData[0]));

		auto effectNode = doc.first_node("effect");

		auto includeNode = effectNode->first_node("include");
		while (includeNode)
		{
			String includeFileName = includeNode->first_attribute("name")->value();
			WString wIncludeFileName;
			ConvertStr_AToW(includeFileName, wIncludeFileName);
			auto includeFileTime = GetEffectsLastedTime(basePath, wIncludeFileName);
			if (includeFileTime.lastWriteTime > fileTime.lastWriteTime)
				fileTime = includeFileTime;
			includeNode = includeNode->next_sibling("include");
		}

		return fileTime;
	}

	Ptr<RenderEffect> RenderEffect::Load(const Ptr<File> & file)
	{
		//Read Data
		if (!file->CanRead())
			return Ptr<RenderEffect>();

		auto buf = MakeBufferedData(file->Size() + 1);
		auto bytesRead = file->Read(buf.get(), file->Size());
		buf[file->Size()] = 0;
		if (bytesRead != file->Size())
			return Ptr<RenderEffect>();

		auto effect = std::make_shared<RenderEffect>();

		effect->_effectName = file->GetName();

		//Load Compiled Data
		auto rawFXFileTime = GetEffectsLastedTime(file->ParentPath(), file->GetName() + L"." + file->GetExtensions());
		auto pcfxFile = std::make_shared<File>(effect->GetCompiledDataSavePath(), FILE_OPEN_READ);
		if (pcfxFile->CanRead())
		{
			auto pcfxFileTime = pcfxFile->GetTime();
			if (pcfxFileTime.lastWriteTime >= rawFXFileTime.lastWriteTime)
				effect->LoadCompiledData(std::make_shared<FileReader>(pcfxFile));
		}
		pcfxFile.reset();

		//Parse XML
		using namespace rapidxml;
		xml_document<> doc;
		doc.parse<0>(reinterpret_cast<char*>(buf.get()));

		//Root Effect Node
		auto effectNode = doc.first_node("effect");
		if (!effectNode)
			return Ptr<RenderEffect>();


		//Add Include File
		auto includeNode = effectNode->first_node("include");
		std::vector< std::shared_ptr<xml_document<>> > includeDocs;
		std::vector< std::unique_ptr<uint8_t[]> > includeBufs;
		while (includeNode)
		{
			String includeFileName = includeNode->first_attribute("name")->value();
			WString wIncludeFileName;
			ConvertStr_AToW(includeFileName, wIncludeFileName);
			File includeFile = File(file->ParentPath() + wIncludeFileName, FILE_OPEN_READ);
			if (includeFile.CanRead())
			{
				auto buf = std::unique_ptr<uint8_t[]>(new uint8_t[includeFile.Size() + 1]);
				includeFile.Read(buf.get(), includeFile.Size());
				buf[includeFile.Size()] = 0;

				auto incDoc = std::make_shared<xml_document<>>();
				//xml_document<> incDoc;
				incDoc->parse<0>(reinterpret_cast<char*>(buf.get()));

				AddInclude(effectNode, includeNode->next_sibling(), incDoc.get());
				auto removeInc = includeNode;
				includeNode = includeNode->next_sibling("include");
				effectNode->remove_node(removeInc);

				includeDocs.push_back(incDoc);
				includeBufs.push_back(std::move(buf));
			}
			else
			{
				includeNode = includeNode->next_sibling("include");
			}
		}

		//GetXMLString(effectNode, effect->_rawFXText);

		std::stringstream ss;

		//Load Headers
		auto headerNode = effectNode->first_node("header");
		while (headerNode)
		{
			ss << headerNode->first_node()->value();
			headerNode = headerNode->next_sibling("header");
		}

		//Load Macros
		auto macroNode = effectNode->first_node("macro");
		while (macroNode)
		{
			MacroDesc macro;
			macro.name = macroNode->first_attribute("name")->value();
			auto macroValueAttri = macroNode->first_attribute("value");
			if (macroValueAttri)
			{
				macro.value = macroValueAttri->value();
			}
			//effect->_globalMacros[macro.name] = macro;
			effect->_effectMacros.push_back(macro);
			effect->_macrosMap[macro.name] = macro;

			macroNode = macroNode->next_sibling("macro");
		}

		SortMacros(effect->_effectMacros);
		effect->_macrosHashKey = HashMacroDescs(effect->_effectMacros);

		//Load Variables
		auto variableNode = effectNode->first_node("variable");
		while (variableNode)
		{
			auto variable = RenderEffectVariable::Load(variableNode, effect->_effectMacros);
			if (variable)
			{
				if (variable->IsConstantBuffer())
				{
					//register cbuffer variables
					auto bufVar = variable->AsConstantBuffer();
					for (uint32_t bufVarIndex = 0; bufVarIndex != bufVar->NumVariables(); ++bufVarIndex)
						effect->_variableMap[bufVar->VariableByIndex(bufVarIndex)->Name()] = bufVar->VariableByIndex(bufVarIndex);
				}

				effect->_variableMap[variable->Name()] = variable;
			}

			variableNode = variableNode->next_sibling("variable");
		}
		for (auto & var : effect->_variableMap)
			if (!var.second->IsScalar())
				ss << var.second->GetCodeString();

		//Load Shader Code
		auto shaderNode = effectNode->first_node("shader");
		while (shaderNode)
		{
			ss << shaderNode->first_node()->value();

			shaderNode = shaderNode->next_sibling("shader");
		}
		effect->_effectCode = ss.str();

		//Load Technique
		auto techniqueNode = effectNode->first_node("technique");
		while (techniqueNode)
		{
			auto technique = RenderTechnique::Load(techniqueNode, effect);
			if (technique)
			{
				effect->_techniques.push_back(technique);
			}

			techniqueNode = techniqueNode->next_sibling("technique");
		}

		//Init Default Shaders
		bool bExtraNeed = false;
		for (auto & var : effect->_variableMap)
		{
			if (var.second->IsConstantBuffer())
				bExtraNeed |= var.second->AsConstantBuffer()->IsExtraMacrosRelevant();
		}

		if (!bExtraNeed)
		{
			effect->InitCompileData();
			effect->InitPassShaderPrograms();
		}

		return effect;
	}

	void RenderEffect::Save()
	{
		auto pcfxFile = std::make_shared<File>(GetCompiledDataSavePath(), FILE_OPEN_WRITE);
		pcfxFile->MakeParentPathDirectory();
		SaveCompiledData(std::make_shared<FileWriter>(pcfxFile));
	}

	Ptr<RenderTechnique> RenderEffect::TechniqueByName(const String & name) const
	{
		for (auto & tech : _techniques)
		{
			if (tech->Name() == name)
				return tech;
		}
		
		return Ptr<RenderTechnique>();
	}

	Ptr<RenderEffectVariable> RenderEffect::VariableByName(const String & name) const
	{
		auto varFind = _variableMap.find(name);
		if (varFind != _variableMap.end())
		{
			bool bExtraRelevant =
					(varFind->second->IsConstantBuffer()		&& varFind->second->AsConstantBuffer()->IsExtraMacrosRelevant())
				||	(varFind->second->IsScalar()				&& varFind->second->AsScalar()->ConstantBuffer().lock()->IsExtraMacrosRelevant());

			if (bExtraRelevant)
				return _effectCompileMap.find(_macrosHashKey)->second.macroRelevantVarsMap.find(name)->second;
			else
				return varFind->second;
		}
		else
		{
			return Ptr<RenderEffectVariable>();
		}
	}

	void RenderEffect::SetExtraMacros(const std::vector<MacroDesc> & macros)
	{
		_extraMacros = macros;
		_macrosMap.clear();
		for (auto & macro : _effectMacros)
			_macrosMap[macro.name] = macro;
		for (auto & macro : _extraMacros)
			_macrosMap[macro.name] = macro;

		InitCompileData();
		InitPassShaderPrograms();
	}

	void RenderEffect::AddExtraMacro(const String & name, const String & value)
	{
		for (auto & macro : _extraMacros)
		{
			if (macro.name == name)
			{
				macro.value = value;
				return;
			}
		}

		for (auto & macro : _effectMacros)
		{
			if (macro.name == name) //Not Change FXFile Defined Macros
				return;
		}

		_extraMacros.push_back({ name, value });
	}

	void RenderEffect::RemoveExtraMacro(const String & name)
	{
		for (auto itr = _extraMacros.begin(); itr != _extraMacros.end(); ++itr)
		{
			if (itr->name == name)
			{
				_extraMacros.erase(itr);
				return;
			}
		}
	}

	void RenderEffect::UpdateData()
	{
		InitCompileData();
		InitPassShaderPrograms();
	}

	const MacroDesc & RenderEffect::MacroByName(const String & name) const
	{
		static MacroDesc emptyRet;

		auto macroFind = _macrosMap.find(name);
		if (macroFind != _macrosMap.end())
			return macroFind->second;
		else
			return emptyRet;
	}

	WString RenderEffect::GetCompiledDataSavePath()
	{
		auto fxResourceBasePath = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->BasePath();
		auto pcfxFilePath = fxResourceBasePath + L"pcfx\\" + _effectName;
#if defined (DEBUG) || defined (_DEBUG)
		pcfxFilePath += L"d";
#endif
		pcfxFilePath += L".pcfx";

		return pcfxFilePath;
	}


	void RenderEffect::InitCompileData()
	{
		auto compileMacros = _effectMacros;
		compileMacros.insert(compileMacros.end(), _extraMacros.begin(), _extraMacros.end());
		SortMacros(compileMacros);
		_macrosHashKey = HashMacroDescs(compileMacros);

		auto & compileData = _effectCompileMap[_macrosHashKey];
		if (compileData.bInit)
			return;

		compileData.macros = compileMacros;
		compileData.macroHashKey = _macrosHashKey;

		std::stringstream ss;
		for (auto & macro : compileMacros)
			ss << "#define " << macro.name << " " << macro.value << std::endl;
		ss << _effectCode;

		for (auto & tech : _techniques)
		{
			for (auto & pass : tech->_pass)
			{
				for (auto & entry : pass->_shadersEntries)
				{
					auto & shaderProgram = compileData.compileDataMap[entry.second];
					if (!shaderProgram)
						shaderProgram = Global::GetRenderEngine()->GetRenderFactory()->CompileShaderProgram(entry.first, ss.str(), entry.second);

					auto & shader = pass->_shaders[entry.first];
					if (!shader)
						shader = std::make_shared<Shader>();
					//shader->SetProgram(shaderProgram);

					for (auto & cbInfo : shaderProgram->GetReflectInfo().buffers)
					{
						auto & cbVar = _variableMap[cbInfo.name];
						if (!cbVar->IsConstantBuffer())
							continue;

						if (cbVar->AsConstantBuffer()->IsExtraMacrosRelevant())
						{
							auto & cbExtra = compileData.macroRelevantVarsMap[cbInfo.name];
							if (!cbExtra)
							{
								cbExtra = cbVar->AsConstantBuffer()->CreateCopy();
								cbExtra->AsConstantBuffer()->InitFromReflectionInfo(cbInfo);

								//Register Sub Variables
								for (int32_t varIndex = 0; varIndex < cbExtra->AsConstantBuffer()->NumVariables(); ++varIndex)
								{
									auto & subVar = cbExtra->AsConstantBuffer()->VariableByIndex(varIndex);
									compileData.macroRelevantVarsMap[subVar->Name()] = subVar;
								}
							}
						}
						else
						{
							if (cbVar->AsConstantBuffer()->GetSize() == 0)
								cbVar->AsConstantBuffer()->InitFromReflectionInfo(cbInfo);
						}
					}
				}
			}
		}

		compileData.bInit = true;
	}

	void RenderEffect::InitPassShaderPrograms()
	{
		auto & compileData = _effectCompileMap[_macrosHashKey];
		for (auto & tech : _techniques)
		{
			for (auto & pass : tech->_pass)
			{
				for (auto & entry : pass->_shadersEntries)
				{
					auto & passShader = pass->_shaders[entry.first];
					auto & program = compileData.compileDataMap[entry.second];

					if (passShader->GetProgram() != program)
						passShader->SetProgram(program);
				}
			}
		}
	}

	void RenderEffect::SaveCompiledData(const Ptr<Writer> & writer)
	{
		writer->Write<int32_t>(static_cast<int32_t>(_effectCompileMap.size()));
		for (auto & effectData : _effectCompileMap)
		{
			writer->Write<uint64_t>(effectData.second.macroHashKey);
			writer->Write<int32_t>(static_cast<int32_t>(effectData.second.macros.size()));
			for (auto & macro : effectData.second.macros)
			{
				writer->WriteString(macro.name);
				writer->WriteString(macro.value);
			}

			writer->Write<int32_t>(static_cast<int32_t>(effectData.second.compileDataMap.size()));
			for (auto & compiledData : effectData.second.compileDataMap)
			{
				writer->WriteString(compiledData.first);
				compiledData.second->Save(writer);
				/*writer->Write<size_t>(compiledData.second->GetCompiledDataSize());
				writer->WriteBytes(compiledData.second->GetCompiledData().get(), compiledData.second->GetCompiledDataSize());*/
			}
		}
	}

	void RenderEffect::LoadCompiledData(const Ptr<Reader> & reader)
	{
		auto effectDataMapSize = reader->Read<int32_t>();
		for (int32_t effectDataIndex = 0; effectDataIndex < effectDataMapSize; ++effectDataIndex)
		{
			auto hashKey = reader->Read<uint64_t>();
			auto & effectData = _effectCompileMap[hashKey];
			effectData.macroHashKey = hashKey;

			auto numMacros = reader->Read<int32_t>();
			for (int32_t macroIndex = 0; macroIndex < numMacros; ++macroIndex)
			{
				String macroName;
				reader->ReadString(macroName);
				String macroValue;
				reader->ReadString(macroValue);
				effectData.macros.push_back({ macroName, macroValue });
			}

			auto numCompiledData = reader->Read<int32_t>();
			for (int32_t compiledDataIndex = 0; compiledDataIndex < numCompiledData; ++compiledDataIndex)
			{
				String entry;
				reader->ReadString(entry);
				effectData.compileDataMap[entry] = ShaderProgram::Load(reader);
			}
		}
	}
}