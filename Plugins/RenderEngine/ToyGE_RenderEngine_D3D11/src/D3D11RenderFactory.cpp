#include "ToyGE\D3D11\D3D11RenderFactory.h"
#include "ToyGE\D3D11\D3D11ShaderProgram.h"
#include "ToyGE\Platform\DllLoader.h"
#include "ToyGE\Platform\DllObj.h"
#include "ToyGE\D3D11\D3D11Texture1D.h"
#include "ToyGE\D3D11\D3D11Texture2D.h"
#include "ToyGE\D3D11\D3D11Texture3D.h"
#include "ToyGE\D3D11\D3D11TextureCube.h"
#include "ToyGE\D3D11\D3D11BlendState.h"
#include "ToyGE\D3D11\D3D11DepthStencilState.h"
#include "ToyGE\D3D11\D3D11RasterizerState.h"
#include "ToyGE\D3D11\D3D11Sampler.h"
#include "ToyGE\D3D11\D3D11RenderBuffer.h"
#include "ToyGE\D3D11\D3D11RenderInput.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Platform\PlatformFactory.h"
#include "ToyGE\D3D11\D3D11TransientBuffer.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	D3D11RenderFactory::D3D11RenderFactory()
	{
		auto & compileDll = Global::GetPlatformFactory()->AcquireDll("d3dCompiler_47.dll");
		_compileFunc = reinterpret_cast<D3DCompileFunc>(compileDll->GetProcAddress("D3DCompile"));
		_reflectFunc = reinterpret_cast<D3DReflectFunc>(compileDll->GetProcAddress("D3DReflect"));
	}

	Ptr<RenderInput> D3D11RenderFactory::CreateRenderInput()
	{
		return std::make_shared<D3D11RenderInput>();
	}

	namespace
	{
		static std::map<ShaderType, String> shaderTargetNameMap = 
		{
			{ SHADER_VS, "vs_5_0" },
			{ SHADER_HS, "hs_5_0" },
			{ SHADER_DS, "ds_5_0" },
			{ SHADER_GS, "gs_5_0" },
			{ SHADER_PS, "ps_5_0" },
			{ SHADER_CS, "cs_5_0" }
		};
	}

	Ptr<ShaderProgram> D3D11RenderFactory::CompileShaderProgram(
		ShaderType shaderType,
		const String & shaderText,
		const String & entryName)
	{
		ID3DBlob *pCode = nullptr;
		ID3DBlob *pError = nullptr;

		uint32_t flag = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#if !(defined (DEBUG) || defined (_DEBUG))
		//flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
		flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		//flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		_compileFunc(shaderText.c_str(), shaderText.size(), 0, nullptr, 0,
			entryName.c_str(), shaderTargetNameMap[shaderType].c_str(), flag, 0, &pCode, &pError);
		/*::D3DCompile(pData, dataSize, 0, pD3DMacros, 0,
			entryName.c_str(), GetTargetName().c_str(), flag, 0, &pCode, &pError);*/

		if (nullptr != pError)
		{
			Logger::Log(reinterpret_cast<char*>(pError->GetBufferPointer()));

			std::stringstream ss;
			auto str = shaderText.c_str();// reinterpret_cast<const char*>(pData);
			for (size_t i = 0; i < shaderText.size(); ++i)
			{
				if (str[i] == '%')
					ss << '%';

				ss << str[i];
			}

			Logger::Log(ss.str().c_str());
			ToyGE_ASSERT_FAIL("Compile error!");
		}

		auto compiledData = MakeBufferedDataShared(pCode->GetBufferSize());
		memcpy(compiledData.get(), pCode->GetBufferPointer(), pCode->GetBufferSize());

		auto ret = this->CreateShaderShaderProgram(shaderType, compiledData, pCode->GetBufferSize());

		pCode->Release();
		if (pError)
			pError->Release();

		return ret;
	}

	Ptr<ShaderProgram> D3D11RenderFactory::CreateShaderShaderProgram(
		ShaderType shaderType,
		const std::shared_ptr<uint8_t> & compiledData,
		size_t dataSize)
	{
		auto ret = std::static_pointer_cast<D3D11ShaderProgram>(this->CreateShaderShaderProgram(shaderType));

		ret->Init(compiledData, dataSize);
		this->InitShaderProgramReflectInfo(ret);

		return ret;
	}

	Ptr<ShaderProgram> D3D11RenderFactory::CreateShaderShaderProgram(ShaderType shaderType)
	{
		Ptr<D3D11ShaderProgram> ret;
		switch (shaderType)
		{
		case SHADER_VS:
		{
			ret = std::make_shared<D3D11VertexShaderProgram>();
			//ret = std::make_shared<Shader>(shaderProgram);
			break;
		}
		case SHADER_GS:
		{
			ret = std::make_shared<D3D11GeometryShaderProgram>();
			//ret = std::make_shared<Shader>(shaderProgram);
			break;
		}
		case SHADER_DS:
		{
			ret = std::make_shared<D3D11DomainShaderProgram>();
			//ret = std::make_shared<Shader>(shaderProgram);
			break;
		}
		case SHADER_HS:
		{
			ret = std::make_shared<D3D11HullShaderProgram>();
			//ret = std::make_shared<Shader>(shaderProgram);
			break;
		}
		case SHADER_PS:
		{
			ret = std::make_shared<D3D11PixelShaderProgram>();
			//ret = std::make_shared<Shader>(shaderProgram);
			break;
		}
		case SHADER_CS:
		{
			ret = std::make_shared<D3D11ComputeShaderProgram>();
			//ret = std::make_shared<Shader>(shaderProgram);
			break;
		}
		default:
			break;
		}

		return ret;
	}

	Ptr<Texture>
		D3D11RenderFactory::CreateTexture
		(const TextureDesc & desc)
	{
		switch (desc.type)
		{
		case ToyGE::TEXTURE_UNDEFINED:
			return Ptr<Texture>();

		case ToyGE::TEXTURE_1D:
			return std::make_shared<D3D11Texture1D>(desc);

		case ToyGE::TEXTURE_2D:
			return std::make_shared<D3D11Texture2D>(desc);

		case ToyGE::TEXTURE_3D:
			return std::make_shared<D3D11Texture3D>(desc);

		case ToyGE::TEXTURE_CUBE:
			return std::make_shared<D3D11TextureCube>(desc);

		default:
			break;
		}

		return Ptr<Texture>();
	}

	Ptr<Texture>
		D3D11RenderFactory::CreateTexture
		(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList)
	{
		switch (desc.type)
		{
		case ToyGE::TEXTURE_UNDEFINED:
			return Ptr<Texture>();

		case ToyGE::TEXTURE_1D:
			return std::make_shared<D3D11Texture1D>(desc, initDataList);

		case ToyGE::TEXTURE_2D:
			return std::make_shared<D3D11Texture2D>(desc, initDataList);

		case ToyGE::TEXTURE_3D:
			return std::make_shared<D3D11Texture3D>(desc, initDataList);

		case ToyGE::TEXTURE_CUBE:
			return std::make_shared<D3D11TextureCube>(desc, initDataList);

		default:
			break;
		}

		return Ptr<Texture>();
	}

	Ptr<RenderBuffer> D3D11RenderFactory::CreateBuffer(const RenderBufferDesc & desc, const void * pInitData)
	{
		return std::make_shared<D3D11RenderBuffer>(desc, pInitData);
	}

	Ptr<TransientBuffer> D3D11RenderFactory::CreateTransientBuffer(int32_t elementSize, int32_t initNumElements, uint32_t bufferBindFlags)
	{
		return std::make_shared<D3D11TransientBuffer>(elementSize, initNumElements, bufferBindFlags);
	}

	Ptr<BlendState> D3D11RenderFactory::CreateBlendState(const BlendStateDesc & desc)
	{
		return std::make_shared<D3D11BlendState>(desc);
	}

	Ptr<DepthStencilState> D3D11RenderFactory::CreateDepthStencilState(const DepthStencilStateDesc & desc)
	{
		return std::make_shared<D3D11DepthStencilState>(desc);
	}

	Ptr<RasterizerState> D3D11RenderFactory::CreateRasterizerState(const RasterizerStateDesc & desc)
	{
		return std::make_shared<D3D11RasterizerState>(desc);
	}

	Ptr<Sampler> D3D11RenderFactory::CreateSampler(const SamplerDesc & desc)
	{
		return std::make_shared<D3D11Sampler>(desc);
	}

	DEFINE_GUID(IID_ID3D11ShaderReflection_46,
		0x8d536ca1, 0x0cca, 0x4956, 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84);
	DEFINE_GUID(IID_ID3D11ShaderReflection_47,
		0x8d536ca1, 0x0cca, 0x4956, 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84);

	void D3D11RenderFactory::InitShaderProgramReflectInfo(const Ptr<ShaderProgram> & shaderProgram)
	{
		if (!shaderProgram)
			return;

		//auto & compiledCode = shaderProgram->GetCompiledData();

		ID3D11ShaderReflection* pReflector = nullptr;
		_reflectFunc(shaderProgram->GetCompiledData().get(), shaderProgram->GetCompiledDataSize(), IID_ID3D11ShaderReflection_47, reinterpret_cast<void**>(&pReflector));
		//::D3DReflect(compiledCode.bufferPointer, compiledCode.bufferSize, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&pReflector));
		if (nullptr == pReflector)
		{
			ToyGE_ASSERT_FAIL("Could not reflect shader");
		}

		D3D11_SHADER_DESC d3dShaderDesc;
		pReflector->GetDesc(&d3dShaderDesc);

		ShaderProgramReflectionInfo reflectInfo;

		reflectInfo.buffers.clear();
		reflectInfo.shaderResources.clear();
		reflectInfo.samplers.clear();

		//constant buffers
		for (uint32_t cbIndex = 0; cbIndex != d3dShaderDesc.ConstantBuffers; ++cbIndex)
		{
			auto d3dCB = pReflector->GetConstantBufferByIndex(cbIndex);
			D3D11_SHADER_BUFFER_DESC d3dShaderBufDesc;
			d3dCB->GetDesc(&d3dShaderBufDesc);
			if (d3dShaderBufDesc.Type != D3D_CT_CBUFFER)
				continue;

			BufferReflectionInfo bufDesc;
			bufDesc.name = d3dShaderBufDesc.Name;
			bufDesc.bytesSize = d3dShaderBufDesc.Size;

			for (uint32_t varIndex = 0; varIndex != d3dShaderBufDesc.Variables; ++varIndex)
			{
				D3D11_SHADER_VARIABLE_DESC d3dBufVarDesc;
				d3dCB->GetVariableByIndex(varIndex)->GetDesc(&d3dBufVarDesc);
				BufferVarReflectionInfo bufVarDesc;
				bufVarDesc.name = d3dBufVarDesc.Name;
				bufVarDesc.bytesOffset = d3dBufVarDesc.StartOffset;
				bufVarDesc.bytesSize = d3dBufVarDesc.Size;

				bufDesc.variables.push_back(bufVarDesc);
			}

			reflectInfo.buffers.push_back(bufDesc);
		}

		//bound resources
		for (uint32_t resIndex = 0; resIndex != d3dShaderDesc.BoundResources; ++resIndex)
		{
			D3D11_SHADER_INPUT_BIND_DESC d3dResDesc;
			pReflector->GetResourceBindingDesc(resIndex, &d3dResDesc);
			BoundResourceReflectionInfo resDesc;
			resDesc.name = d3dResDesc.Name;
			resDesc.bindPoint = d3dResDesc.BindPoint;

			switch (d3dResDesc.Type)
			{
			case D3D_SIT_CBUFFER:
				for (auto & bufInfo : reflectInfo.buffers)
					if (bufInfo.name == resDesc.name)
						bufInfo.bindPoint = resDesc.bindPoint;
				break;

			case D3D_SIT_TEXTURE:
			case D3D_SIT_STRUCTURED:
				reflectInfo.shaderResources.push_back(resDesc);
				break;

			case D3D_SIT_UAV_RWTYPED:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				reflectInfo.uavs.push_back(resDesc);
				break;

			case D3D_SIT_SAMPLER:
				reflectInfo.samplers.push_back(resDesc);
				break;

			default:
				break;
			}
		}

		shaderProgram->SetReflectInfo(reflectInfo);
	}
}