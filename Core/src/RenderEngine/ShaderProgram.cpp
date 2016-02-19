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
		static void SaveBufferReflectionInfo(const BoundConstantBuffer & cbInfo, const Ptr<Writer> & writer)
		{
			writer->WriteString(cbInfo.name);
			writer->Write(static_cast<int32_t>(cbInfo.bytesSize));
			writer->Write(cbInfo.boundIndex);

			writer->Write<int32_t>(static_cast<int32_t>(cbInfo.variables.size()));
			for (auto & varInfo : cbInfo.variables)
			{
				writer->WriteString(varInfo.name);
				writer->Write(static_cast<int32_t>(varInfo.bytesOffset));
				writer->Write(static_cast<int32_t>(varInfo.bytesSize));
			}
		}

		static void LoadBufferReflectionInfo(const Ptr<Reader> & reader, BoundConstantBuffer & outCBInfo)
		{
			reader->ReadString(outCBInfo.name);
			outCBInfo.bytesSize = reader->Read<int32_t>();
			outCBInfo.boundIndex = reader->Read<int32_t>();

			auto numVars = reader->Read<int32_t>();
			outCBInfo.variables.resize(numVars);
			for (auto & varInfo : outCBInfo.variables)
			{
				reader->ReadString(varInfo.name);
				varInfo.bytesOffset = reader->Read<int32_t>();
				varInfo.bytesSize = reader->Read<int32_t>();
			}
		}

		static void SaveBoundResourceReflectionInfo(const BoundResource & rsInfo, const Ptr<Writer> & writer)
		{
			writer->WriteString(rsInfo.name);
			writer->Write(rsInfo.boundIndex);
		}

		static void LoadBoundResourceReflectionInfo(const Ptr<Reader> & reader, BoundResource & outRsInfo)
		{
			reader->ReadString(outRsInfo.name);
			outRsInfo.boundIndex = reader->Read<int32_t>();
		}
	}

	void ShaderProgram::Init()
	{
		RenderResource::Init();

		Global::GetRenderEngine()->GetRenderFactory()->GetShaderProgramResourceMap(GetCompiledData(), GetCompiledDataSize(), _resourceMap);
	}
}