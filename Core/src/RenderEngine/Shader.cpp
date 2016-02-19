#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "mcpp.h"

namespace ToyGE
{
	String Shader::_shaderFilesBasePath;

	void Shader::Init()
	{
		// Clear
		_variablesMap.clear();

		auto & resourceMap = _shaderProgram->GetResourceMap();

		// Constant buffers
		{
			int32_t maxBoundIndex = -1;
			for (auto & buf : resourceMap.constantBuffers)
				maxBoundIndex = std::max<int32_t>(maxBoundIndex, buf.boundIndex);
			_cbVars.resize(maxBoundIndex + 1);
		}

		for (auto & buf : resourceMap.constantBuffers)
		{
			auto cbVar = std::make_shared<ShaderCBVariable>();
			cbVar->name = buf.name;
			cbVar->shader = shared_from_this();
			cbVar->boundIndex = buf.boundIndex;
			cbVar->bytesSize = buf.bytesSize;

			// Members
			for (auto & member : buf.variables)
			{
				auto memberVar = std::make_shared<ShaderCBMemberVariable>();
				memberVar->name = member.name;
				memberVar->shader = shared_from_this();
				memberVar->bytesOffset = member.bytesOffset;
				memberVar->bytesSize = member.bytesSize;
				memberVar->outerCB = cbVar;

				cbVar->memberVars.push_back(memberVar);

				// Register member
				_variablesMap[memberVar->name] = memberVar;
			}
			// Register cb
			_variablesMap[cbVar->name] = cbVar;
			_cbVars[cbVar->boundIndex] = cbVar;

			// Create hardware cb if not shared
			String sharedPostfix = "_shared";
			if (cbVar->name.rfind(sharedPostfix) == String::npos || cbVar->name.rfind(sharedPostfix) != cbVar->name.size() - sharedPostfix.size())
			{
				cbVar->data = MakeBufferedDataShared(static_cast<size_t>(cbVar->bytesSize));

				auto cb = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
				RenderBufferDesc bufDesc;
				bufDesc.bindFlag = BUFFER_BIND_CONSTANT;
				bufDesc.cpuAccess = CPU_ACCESS_WRITE;
				bufDesc.elementSize = cbVar->bytesSize;
				bufDesc.numElements = 1;
				bufDesc.bStructured = false;
				cb->SetDesc(bufDesc);
				cb->Init(nullptr);

				cbVar->Set(cb);
			}
		}

		// SRVs
		{
			int32_t maxBoundIndex = -1;
			for (auto & srv : resourceMap.srvs)
				maxBoundIndex = std::max<int32_t>(maxBoundIndex, srv.boundIndex);
			_srvVars.resize(maxBoundIndex + 1);
		}

		for (auto & srv : resourceMap.srvs)
		{
			auto srvVar = std::make_shared<ShaderSRVVariable>();
			srvVar->name = srv.name;
			srvVar->shader = shared_from_this();
			srvVar->boundIndex = srv.boundIndex;

			// Register srv
			_variablesMap[srvVar->name] = srvVar;
			_srvVars[srvVar->boundIndex] = srvVar;
		}

		// UAVs
		{
			int32_t maxBoundIndex = -1;
			for (auto & uav : resourceMap.uavs)
				maxBoundIndex = std::max<int32_t>(maxBoundIndex, uav.boundIndex);
			_uavVars.resize(maxBoundIndex + 1);
		}

		for (auto & uav : resourceMap.uavs)
		{
			auto uavVar = std::make_shared<ShaderUAVVariable>();
			uavVar->name = uav.name;
			uavVar->shader = shared_from_this();
			uavVar->boundIndex = uav.boundIndex;

			// Register uav
			_variablesMap[uavVar->name] = uavVar;
			_uavVars[uavVar->boundIndex] = uavVar;
		}

		// Samplers
		{
			int32_t maxBoundIndex = -1;
			for (auto & sampler : resourceMap.samplers)
				maxBoundIndex = std::max<int32_t>(maxBoundIndex, sampler.boundIndex);
			_samplerVars.resize(maxBoundIndex + 1);
		}

		for (auto & sampler : resourceMap.samplers)
		{
			auto samplerVar = std::make_shared<ShaderSamplerVariable>();
			samplerVar->name = sampler.name;
			samplerVar->shader = shared_from_this();
			samplerVar->boundIndex = sampler.boundIndex;

			// Register sampler
			_variablesMap[samplerVar->name] = samplerVar;
			_samplerVars[samplerVar->boundIndex] = samplerVar;
		}
	}

	Ptr<ShaderVariable> Shader::Variable(const String & name) const
	{
		auto varFind = _variablesMap.find(name);
		if (varFind != _variablesMap.end())
			return varFind->second;
		else
		{
			//Logger::LogLine("warning> shader variable not found shader[%s] variable[%s]", _name.c_str(), name.c_str());
			return nullptr;
		}
	}

	void Shader::Flush()
	{
		/*for (auto & varPair : _variablesMap)
		{
			auto var = varPair.second;
			var->Flush();
		}*/

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetShader(_shaderProgram);

		// SRVs
		{
			std::vector<Ptr<ShaderResourceView>> srvs;
			for (auto & var : _srvVars)
			{
				srvs.push_back(var->srv);
				var->Clear();
			}
			rc->SetSRVs(_shaderProgram->GetType(), srvs);
		}

		// UAVs
		{
			std::vector<Ptr<UnorderedAccessView>> uavs;
			std::vector<int32_t> uavInitalCounts;
			for (auto & var : _uavVars)
			{
				if (!var)
					continue;

				uavs.push_back(var->uav);
				uavInitalCounts.push_back(var->initialCount);
				var->Clear();
			}
			rc->SetUAVs(_shaderProgram->GetType(), uavs, uavInitalCounts);
		}

		// CBs
		{
			std::vector<Ptr<RenderBuffer>> cbs;
			for (auto & var : _cbVars)
			{
				// Update cb
				if (var->bDirty && var->cb && var->data)
				{
					auto mappedData = var->cb->Map(MapType::MAP_WRITE_DISCARD);
					memcpy_s(mappedData.pData, var->bytesSize, var->data.get(), var->bytesSize);
					var->cb->UnMap();
				}
				var->bDirty = false;

				cbs.push_back(var->cb);
			}
			rc->SetCBs(_shaderProgram->GetType(), cbs);
		}

		// Samplers
		{
			std::vector<Ptr<Sampler>> samplers;
			for (auto & var : _samplerVars)
			{
				samplers.push_back(var->sampler);
				var->Clear();
			}
			rc->SetSamplers(_shaderProgram->GetType(), samplers);
		}
	}


	static int GetFileContents(void * userData, const char * fileName, const char ** outContents, size_t * outContentSize)
	{
		auto file = Global::GetPlatform()->CreatePlatformFile(fileName, FILE_OPEN_READ);

		if (!file->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot open shader file! %s", fileName);
			return 0;
		}

		if (outContents)
		{
			auto fileSize = file->Size();

			char * data = new char[fileSize + 2];
			file->Read(data, fileSize);

			for (size_t i = 0; i < fileSize; ++i)
			{
				if (data[i] == '\r' && i < fileSize - 1 && data[i + 1] == '\n')
				{
					data[i] = ' ';
				}
			}
			data[fileSize] = ' ';
			data[fileSize + 1] = '\n';

			*outContents = data;
			*outContentSize = fileSize + 2;

			return fileSize > 0;
		}

		return 1;
	}


//	std::vector<ShaderMetaType*> ShaderMetaType::_shaderMetaTypeList;

	ShaderMetaType::ShaderMetaType(ShaderType shaderType, const String & shaderFileName, const String & entryName, const String & shaderName, ShaderModel minimunShaderModel)
		: _shaderType(shaderType),
		_shaderFileName(shaderFileName),
		_entryName(entryName),
		_shaderName(shaderName),
		_minimumShaderModel(minimunShaderModel)
	{
		GetShaderMetaTypeList().push_back(this);
	}

	bool ShaderMetaType::IsAvailable() const
	{
		return RenderEngine::GetShaderModel() >= _minimumShaderModel;
	}

	Ptr<Shader> ShaderMetaType::FindOrCreate(const std::map<String, String> & macroDefines)
	{
		if (!IsAvailable())
		{
			ToyGE_LOG(LT_WARNING, "Unsupported shader model! shader model=%s request minimun=%s", 
				GetShaderModelName(RenderEngine::GetShaderModel()), GetShaderModelName(_minimumShaderModel));
			return nullptr;
		}

		if (!_bCacheLoaded)
			LoadCache();

		// Hash macros
		std::stringstream ss;
		for (auto & macroPair : macroDefines)
			ss << macroPair.first << macroPair.second;
		auto hashKey = Hash(ss.str().c_str(), ss.str().size());

		auto & shader = _shaderMap[hashKey][RenderEngine::GetShaderModel()];
		if (!shader)
		{
			// Create new shader
			shader = std::make_shared<Shader>();

			// Preprocess shader code
			auto filePath = (Shader::GetShaderFilesBasePath().size() > 0 ? Shader::GetShaderFilesBasePath() + "/" : "")
				+ _shaderFileName + Shader::GetShaderFileExtension();

			ToyGE_LOG(LT_INFO, "Compiling shader... %s", filePath.c_str());

			std::stringstream optionsSS;
			optionsSS << " -P ";
			for (auto & macroPair : macroDefines)
				optionsSS << " -D" << macroPair.first << "=" << macroPair.second;

			char * outCode = nullptr;
			char * outErrors = nullptr;

			file_loader fileLoader;
			fileLoader.get_file_contents = &GetFileContents;
			fileLoader.user_data = nullptr;

			mcpp_run(optionsSS.str().c_str(), filePath.c_str(), &outCode, &outErrors, fileLoader);

			//Convert from LF to CR+LF
			for (int32_t i = 0; outCode[i]; ++i)
			{
				if (outCode[i] == '\n' && i > 0 && outCode[i - 1] == ' ')
					outCode[i - 1] = '\r';
			}

			if (outErrors)
			{
				ToyGE_LOG(LT_RAW, outErrors);
			}

			// Compile shader
			auto reFactory = Global::GetRenderEngine()->GetRenderFactory();
			auto shaderProgram = reFactory->CreateShaderShaderProgram(_shaderType);
			if (!reFactory->CompileShader(_shaderType, std::move(outCode), _entryName, shaderProgram->_compiledData, shaderProgram->_compiledDataSize))
			{
				ToyGE_ASSERT_FAIL("Fatal runtime error!");
			}
			shaderProgram->Init();
			shader->SetProgram(shaderProgram);
			shader->SetName(_shaderName);

			shader->Init();

			_bNeedUpdateCacheFile = true;

			ToyGE_LOG(LT_INFO, "Compiling shader finished! %s", filePath.c_str());
		}

		return shader;
	}

	FileTime GetShaderLastWriteTime(const String & filePath, std::set<String> & testedFilePath)
	{
		auto shaderFilePath = (Shader::GetShaderFilesBasePath().size() > 0 ? Shader::GetShaderFilesBasePath() + "/" : "")
			+ filePath;
		auto shaderFile = Global::GetPlatform()->CreatePlatformFile(shaderFilePath, FILE_OPEN_READ);
		if (!shaderFile->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot open shader file! %s", shaderFilePath.c_str());
			return { 0 };
		}

		FileTime resultTime = shaderFile->GetTime();

		String shaderCode(shaderFile->Size(), 0);
		shaderFile->Read(&shaderCode[0], shaderFile->Size());
		size_t index = 0;
		String includePrefix = "#include";
		while ((index = shaderCode.find(includePrefix, index)) != String::npos)
		{
			auto begin = shaderCode.find('"', index) + 1;
			auto end = shaderCode.find('"', begin);
			auto includeFilePath = shaderCode.substr(begin, end - begin);

			if (testedFilePath.count(includeFilePath) == 0)
			{
				testedFilePath.insert(includeFilePath);
				auto time = GetShaderLastWriteTime(includeFilePath, testedFilePath);
				if (time.lastWriteTime > resultTime.lastWriteTime)
					resultTime = time;
			}

			index += includeFilePath.size();
		}
		return resultTime;
	}

	void ShaderMetaType::LoadCache()
	{
		_bCacheLoaded = true;

		auto cacheFilePath = (Shader::GetShaderFilesBasePath().size() > 0 ? Shader::GetShaderFilesBasePath() + "/" : "")
			+ "Cache/" + _shaderName + Shader::GetShaderCacheFileExtension();

		if (!Global::GetPlatform()->FileExists(cacheFilePath))
			return;

		auto cacheFile = Global::GetPlatform()->CreatePlatformFile(cacheFilePath, FILE_OPEN_READ);
		if (!cacheFile->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot open shader cache file! %s", cacheFilePath.c_str());
			return;
		}

		// Check if shader file is new, which needs recompilation
		std::set<String> testedFilePath;
		auto cacheFileTime = cacheFile->GetTime();
		if (GetShaderLastWriteTime(_shaderFileName + Shader::GetShaderFileExtension(), testedFilePath).lastWriteTime > cacheFileTime.lastWriteTime)
			return;

		auto reader = std::make_shared<FileReader>(cacheFile);

		auto reFactory = Global::GetRenderEngine()->GetRenderFactory();

		auto mapSize = reader->Read<int32_t>();
		for (int32_t i = 0; i < mapSize; ++i)
		{
			auto key = reader->Read<uint64_t>();
			for (auto & shader : _shaderMap[key])
			{
				auto compiledDataSize = reader->Read<int32_t>();
				if (compiledDataSize <= 0)
					continue;

				shader = std::make_shared<Shader>();

				auto shaderProgram = reFactory->CreateShaderShaderProgram(_shaderType);
				shaderProgram->_compiledDataSize = compiledDataSize;
				shaderProgram->_compiledData = MakeBufferedDataShared((size_t)compiledDataSize);
				reader->ReadBytes(shaderProgram->_compiledData.get(), compiledDataSize);
				//reFactory->GetShaderProgramResourceMap(shaderProgram->GetCompiledData(), shaderProgram->GetCompiledDataSize(), shaderProgram->_resourceMap);
				shaderProgram->Init();
				shader->SetProgram(shaderProgram);
				shader->SetName(_shaderName);

				shader->Init();
			}
		}
	}

	void ShaderMetaType::SaveCache()
	{
		auto cacheFilePath = (Shader::GetShaderFilesBasePath().size() > 0 ? Shader::GetShaderFilesBasePath() + "/" : "")
			+ "Cache/" + _shaderName + Shader::GetShaderCacheFileExtension();

		Global::GetPlatform()->MakeDirectoryRecursively(ParentPath(cacheFilePath));
		auto cacheFile = Global::GetPlatform()->CreatePlatformFile(cacheFilePath, FILE_OPEN_WRITE);
		if (!cacheFile->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot save shader cache! %s", cacheFilePath.c_str());
			return;
		}

		auto writer = std::make_shared<FileWriter>(cacheFile);

		writer->Write<int32_t>((int32_t)_shaderMap.size());
		for (auto & shaderMapPair : _shaderMap)
		{
			writer->Write<uint64_t>(shaderMapPair.first);
			for (auto & shader : shaderMapPair.second)
			{
				if (shader)
				{
					writer->Write<int32_t>(shader->GetProgram()->GetCompiledDataSize());
					writer->WriteBytes(shader->GetProgram()->GetCompiledData().get(), (size_t)shader->GetProgram()->GetCompiledDataSize());
				}
				else
					writer->Write<int32_t>(0);
			}
		}
	}
}