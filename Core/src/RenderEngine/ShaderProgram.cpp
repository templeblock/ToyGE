#include "ToyGE\RenderEngine\ShaderProgram.h"
#include "ToyGE\Kernel\IOHelper.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	namespace
	{
		static void SaveBufferReflectionInfo(const BufferReflectionInfo & bufInfo, const Ptr<Writer> & writer)
		{
			writer->WriteString(bufInfo.name);
			writer->Write(static_cast<uint64_t>(bufInfo.bytesSize));
			writer->Write(bufInfo.bindPoint);

			writer->Write<int32_t>(static_cast<int32_t>(bufInfo.variables.size()));
			for (auto & varInfo : bufInfo.variables)
			{
				writer->WriteString(varInfo.name);
				writer->Write(static_cast<uint64_t>(varInfo.bytesOffset));
				writer->Write(static_cast<uint64_t>(varInfo.bytesSize));
			}
		}

		static void LoadBufferReflectionInfo(const Ptr<Reader> & reader, BufferReflectionInfo & outBufInfo)
		{
			reader->ReadString(outBufInfo.name);
			outBufInfo.bytesSize = reader->Read<uint64_t>();
			outBufInfo.bindPoint = reader->Read<int32_t>();

			auto numVars = reader->Read<int32_t>();
			outBufInfo.variables.resize(numVars);
			for (auto & varInfo : outBufInfo.variables)
			{
				reader->ReadString(varInfo.name);
				varInfo.bytesOffset = reader->Read<uint64_t>();
				varInfo.bytesSize = reader->Read<uint64_t>();
			}
		}

		static void SaveBoundResourceReflectionInfo(const BoundResourceReflectionInfo & rsInfo, const Ptr<Writer> & writer)
		{
			writer->WriteString(rsInfo.name);
			writer->Write(rsInfo.bindPoint);
		}

		static void LoadBoundResourceReflectionInfo(const Ptr<Reader> & reader, BoundResourceReflectionInfo & outRsInfo)
		{
			reader->ReadString(outRsInfo.name);
			outRsInfo.bindPoint = reader->Read<int32_t>();
		}
	}

	ShaderProgram::ShaderProgram(ShaderType type)
		: _type(type)
	{

	}

	void ShaderProgram::Save(const Ptr<Writer> & writer) const
	{
		writer->Write(_type);
		writer->Write(static_cast<uint64_t>(_compiledDataSize));
		writer->WriteBytes(_compiledData.get(), _compiledDataSize);

		writer->Write<int32_t>(static_cast<int32_t>(_reflectInfo.buffers.size()));
		for (auto & info : _reflectInfo.buffers)
			SaveBufferReflectionInfo(info, writer);

		writer->Write<int32_t>(static_cast<int32_t>(_reflectInfo.shaderResources.size()));
		for (auto & info : _reflectInfo.shaderResources)
			SaveBoundResourceReflectionInfo(info, writer);

		writer->Write<int32_t>(static_cast<int32_t>(_reflectInfo.uavs.size()));
		for (auto & info : _reflectInfo.uavs)
			SaveBoundResourceReflectionInfo(info, writer);

		writer->Write<int32_t>(static_cast<int32_t>(_reflectInfo.samplers.size()));
		for (auto & info : _reflectInfo.samplers)
			SaveBoundResourceReflectionInfo(info, writer);
	}

	Ptr<ShaderProgram> ShaderProgram::Load(const Ptr<Reader> & reader)
	{
		auto type = reader->Read<ShaderType>();
		auto shaderProgram = Global::GetRenderEngine()->GetRenderFactory()->CreateShaderShaderProgram(type);
		shaderProgram->DoLoad(reader);

		return shaderProgram;
	}

	void ShaderProgram::DoLoad(const Ptr<Reader> & reader)
	{
		_compiledDataSize = reader->Read<uint64_t>();
		_compiledData = MakeBufferedDataShared(_compiledDataSize);
		reader->ReadBytes(_compiledData.get(), _compiledDataSize);

		int32_t size;

		size = reader->Read<int32_t>();
		_reflectInfo.buffers.resize(size);
		for (auto & info : _reflectInfo.buffers)
			LoadBufferReflectionInfo(reader, info);

		size = reader->Read<int32_t>();
		_reflectInfo.shaderResources.resize(size);
		for (auto & info : _reflectInfo.shaderResources)
			LoadBoundResourceReflectionInfo(reader, info);

		size = reader->Read<int32_t>();
		_reflectInfo.uavs.resize(size);
		for (auto & info : _reflectInfo.uavs)
			LoadBoundResourceReflectionInfo(reader, info);

		size = reader->Read<int32_t>();
		_reflectInfo.samplers.resize(size);
		for (auto & info : _reflectInfo.samplers)
			LoadBoundResourceReflectionInfo(reader, info);
	}
}