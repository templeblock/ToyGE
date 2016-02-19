#pragma once
#ifndef SHADER_H
#define SHADER_H

#include "ToyGE\RenderEngine\RenderResource.h"

#include <boost\noncopyable.hpp>

namespace ToyGE
{
	class ShaderProgram;
	class RenderBuffer;
	class Sampler;

	/**
	 * Varaibles of shader
	 */
	class TOYGE_CORE_API ShaderVariable : public StaticCastable
	{
	public:
		String name;
		std::weak_ptr<class Shader> shader;

		// Release the reference of render resource
		virtual void Clear() = 0;
	};

	class TOYGE_CORE_API ShaderBoundVariable : public ShaderVariable
	{
	public:
		int32_t boundIndex;
	};

	class TOYGE_CORE_API ShaderCBVariable : public ShaderBoundVariable
	{
	public:
		int32_t bytesSize;
		std::vector<Ptr<class ShaderCBMemberVariable>> memberVars;
		std::shared_ptr<uint8_t> data;
		Ptr<RenderBuffer> cb;
		bool bDirty;

		ShaderCBVariable()
			: bDirty(true)
		{

		}

		void Set(const Ptr<RenderBuffer> & value) { cb = value; }

		virtual void Clear() override { cb = nullptr; }

	};

	class TOYGE_CORE_API ShaderCBMemberVariable : public ShaderVariable
	{
	public:
		int32_t bytesOffset;
		int32_t bytesSize;
		std::weak_ptr<class ShaderCBVariable> outerCB;

		void Set(const void * pData, int32_t dataSize)
		{
			if (memcmp(outerCB.lock()->data.get() + bytesOffset, pData, dataSize) != 0)
			{
				memcpy_s(outerCB.lock()->data.get() + bytesOffset, bytesSize, pData, dataSize);
				outerCB.lock()->bDirty = true;
			}
		}

		template<class Type>
		void Set(const Type & value)
		{
			Set(&value, sizeof(Type));
		}

		virtual void Clear() override {};

	};

	class TOYGE_CORE_API ShaderSRVVariable : public ShaderBoundVariable
	{
	public:
		Ptr<ShaderResourceView> srv;

		void Set(const Ptr<ShaderResourceView> & value) { srv = value; }

		virtual void Clear() override { srv = nullptr; }

	};

	class TOYGE_CORE_API ShaderUAVVariable : public ShaderBoundVariable
	{
	public:
		Ptr<UnorderedAccessView> uav;
		int32_t initialCount;

		void Set(const Ptr<UnorderedAccessView> & value, int32_t uavInitialCount) 
		{ 
			uav = value; 
			initialCount = uavInitialCount; 
		}

		virtual void Clear() override { uav = nullptr; }

	};

	class TOYGE_CORE_API ShaderSamplerVariable : public ShaderBoundVariable
	{
	public:
		Ptr<Sampler> sampler;

		void Set(const Ptr<Sampler> & value) { sampler = value; }

		virtual void Clear() override { sampler = nullptr; }

	};


	/**
	 * Shader
	 */
	class TOYGE_CORE_API ShaderMetaType : boost::noncopyable
	{
	public:
		ShaderMetaType(ShaderType shaderType, const String & shaderFileName, const String & entryName, const String & shaderName, ShaderModel minimunShaderModel);

		virtual ~ShaderMetaType() = default;

		CLASS_GET(ShaderFileName, String, _shaderFileName);

		CLASS_GET(ShaderType, ShaderType, _shaderType);

		CLASS_GET(EntryName, String, _entryName);

		Ptr<Shader> FindOrCreate(const std::map<String, String> & macroDefines = {});

		bool IsAvailable() const;

		static std::vector<ShaderMetaType*> & GetShaderMetaTypeList()
		{
			static std::vector<ShaderMetaType*> _shaderMetaTypeList;
			return _shaderMetaTypeList;
		}

		void LoadCache();

		void SaveCache();

		void UpdateFileCache()
		{
			if (_bNeedUpdateCacheFile)
				SaveCache();
		}

	protected:
		ShaderType _shaderType;
		String _shaderFileName;
		String _entryName;
		String _shaderName;
		ShaderModel _minimumShaderModel;
		bool _bCacheLoaded = false;
		bool _bNeedUpdateCacheFile = false;
		std::map< uint64_t, std::array<Ptr<class Shader>, ShaderModelNum::NUM> > _shaderMap;

	};

	class TOYGE_CORE_API Shader : public std::enable_shared_from_this<Shader>
	{
	public:
		Shader() {};

		virtual ~Shader() = default;

		void Init();

		CLASS_SET(Name, String, _name);
		CLASS_GET(Name, String, _name);

		CLASS_SET(Program, Ptr<ShaderProgram>, _shaderProgram);
		CLASS_GET(Program, Ptr<ShaderProgram>, _shaderProgram);

		// Get variable by name
		Ptr<ShaderVariable> Variable(const String & name) const;

		template<class T>
		void SetScalar(const String & varName, const T & value)
		{
			auto var = Variable(varName);
			if (var)
				Variable(varName)->Cast<ShaderCBMemberVariable>()->Set<T>(value);
		}
		void SetScalar(const String & varName, const void * pData, int32_t dataSize)
		{
			auto var = Variable(varName);
			if(var)
				var->Cast<ShaderCBMemberVariable>()->Set(pData, dataSize);
		}
		void SetSRV(const String & varName, const Ptr<ShaderResourceView> & srv)
		{
			auto var = Variable(varName);
			if (var)
				var->Cast<ShaderSRVVariable>()->Set(srv);
		}
		void SetUAV(const String & varName, const Ptr<UnorderedAccessView> & uav, int32_t uavInitialCount = 0)
		{
			auto var = Variable(varName);
			if (var)
				var->Cast<ShaderUAVVariable>()->Set(uav, uavInitialCount);
		}
		void SetSampler(const String & varName, const Ptr<Sampler> & sampelr)
		{
			auto var = Variable(varName);
			if (var)
				var->Cast<ShaderSamplerVariable>()->Set(sampelr);
		}
		void SetCB(const String & varName, const Ptr<RenderBuffer> & cb)
		{
			auto var = Variable(varName);
			if (var)
				var->Cast<ShaderCBVariable>()->Set(cb);
		}
		
		// Update variables to hardware and clear except cbs
		void Flush();

		static void SetShaderFilesBasePath(const String & path) { _shaderFilesBasePath = path; };
		static const String & GetShaderFilesBasePath() { return _shaderFilesBasePath; };

		static const String & GetShaderFileExtension()
		{
			static String ex = ".tshader";
			return ex;
		}
		static const String & GetShaderCacheFileExtension()
		{
			static String ex = ".tscache";
			return ex;
		}


		template<class ShaderType>
		static Ptr<Shader> FindOrCreate(const std::map<String, String> & macroDefines = {})
		{
			return ShaderType::GetMetaType().FindOrCreate(macroDefines);
		}

	private:
		String _name;
		Ptr<ShaderProgram> _shaderProgram;
		std::map<String, Ptr<ShaderVariable>>	_variablesMap;
		std::vector<Ptr<ShaderSRVVariable>>		_srvVars;
		std::vector<Ptr<ShaderUAVVariable>>		_uavVars;
		std::vector<Ptr<ShaderCBVariable>>		_cbVars;
		std::vector<Ptr<ShaderSamplerVariable>> _samplerVars;

		static String _shaderFilesBasePath;
	};

#define DECLARE_SHADER(Prefix, ShaderName, ShaderType, ShaderFileName, EntryName, MinumunShaderModel) \
Prefix \
class ShaderName\
{\
public:\
	static ShaderMetaType & GetMetaType() \
	{\
		static ShaderMetaType _metaType(ShaderType, ShaderFileName, EntryName, #ShaderName, MinumunShaderModel);\
		return _metaType; \
	}\
};

}

#endif