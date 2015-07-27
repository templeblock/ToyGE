#pragma once
#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{
	class TOYGE_CORE_API ShaderProgram
	{
	public:
		//ShaderProgram(ShaderType type, const std::shared_ptr<uint8_t> & compiledData, size_t compiledDataSize)
		//	: _type(type), _compiledData(compiledData), _compiledDataSize(compiledDataSize)
		//{
		//}

		ShaderProgram(ShaderType type);

		virtual ~ShaderProgram() = default;

		const ShaderType Type() const
		{
			return _type;
		}

		CLASS_GET(ReflectInfo, ShaderProgramReflectionInfo, _reflectInfo);
		CLASS_SET(ReflectInfo, ShaderProgramReflectionInfo, _reflectInfo);

		std::shared_ptr<const uint8_t> GetCompiledData() const
		{
			return _compiledData;
		}

		size_t GetCompiledDataSize() const
		{
			return _compiledDataSize;
		}

		virtual void Save(const Ptr<Writer> & writer) const;

		static Ptr<ShaderProgram> Load(const Ptr<Reader> & reader);

		/*const Blob & GetCompiledCode() const
		{
			return _compiledCode;
		}*/

	protected:
		ShaderType _type;
		std::shared_ptr<uint8_t> _compiledData;
		size_t _compiledDataSize;

		ShaderProgramReflectionInfo _reflectInfo;
		//Blob _compiledCode;

		virtual void DoLoad(const Ptr<Reader> & reader);
	};
}

#endif