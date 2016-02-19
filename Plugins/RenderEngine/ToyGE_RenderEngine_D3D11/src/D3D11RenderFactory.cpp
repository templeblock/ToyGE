#include "ToyGE\D3D11\D3D11RenderFactory.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Platform\DllLoader.h"
#include "ToyGE\D3D11\D3D11ShaderProgram.h"
#include "ToyGE\D3D11\D3D11Texture1D.h"
#include "ToyGE\D3D11\D3D11Texture2D.h"
#include "ToyGE\D3D11\D3D11Texture3D.h"
#include "ToyGE\D3D11\D3D11BlendState.h"
#include "ToyGE\D3D11\D3D11DepthStencilState.h"
#include "ToyGE\D3D11\D3D11RasterizerState.h"
#include "ToyGE\D3D11\D3D11Sampler.h"
#include "ToyGE\D3D11\D3D11RenderBuffer.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\D3D11\D3D11TransientBuffer.h"

namespace ToyGE
{
	D3D11RenderFactory::D3D11RenderFactory()
	{
		auto & compileDll = Global::GetPlatform()->FindDll("d3dCompiler_47.dll");
		if (!compileDll)
		{
			ToyGE_LOG(LT_ERROR, "Cannot find module=%s!", "d3dCompiler_47.dll");
			ToyGE_ASSERT_FAIL("Engine init fatal error!");
		}

		_compileFunc = reinterpret_cast<D3DCompileFunc>(compileDll->GetProcAddress("D3DCompile"));
		_reflectFunc = reinterpret_cast<D3DReflectFunc>(compileDll->GetProcAddress("D3DReflect"));
	}

	Ptr<ShaderProgram> D3D11RenderFactory::CreateShaderShaderProgram(ShaderType shaderType)
	{
		Ptr<D3D11ShaderProgram> ret;
		switch (shaderType)
		{
		case SHADER_VS:
		{
			ret = std::make_shared<D3D11VertexShaderProgram>();
			break;
		}
		case SHADER_GS:
		{
			ret = std::make_shared<D3D11GeometryShaderProgram>();
			break;
		}
		case SHADER_DS:
		{
			ret = std::make_shared<D3D11DomainShaderProgram>();
			break;
		}
		case SHADER_HS:
		{
			ret = std::make_shared<D3D11HullShaderProgram>();
			break;
		}
		case SHADER_PS:
		{
			ret = std::make_shared<D3D11PixelShaderProgram>();
			break;
		}
		case SHADER_CS:
		{
			ret = std::make_shared<D3D11ComputeShaderProgram>();
			break;
		}
		default:
			break;
		}
		if(ret)
			ret->Register();

		return ret;
	}

	Ptr<Texture> D3D11RenderFactory::CreateTexture(TextureType type)
	{
		Ptr<Texture> tex;
		switch (type)
		{
		case ToyGE::TEXTURE_UNDEFINED:
			return nullptr;

		case ToyGE::TEXTURE_1D:
			tex = std::make_shared<D3D11Texture1D>();
			break;

		case ToyGE::TEXTURE_2D:
			tex = std::make_shared<D3D11Texture2D>();
			break;

		case ToyGE::TEXTURE_3D:
			tex = std::make_shared<D3D11Texture3D>();
			break;

		default:
			break;
		}
		if (tex)
			tex->Register();

		return tex;
	}

	Ptr<RenderBuffer> D3D11RenderFactory::CreateBuffer()
	{
		auto buf = std::make_shared<D3D11RenderBuffer>();
		buf->Register();
		return buf;
	}

	Ptr<VertexBuffer> D3D11RenderFactory::CreateVertexBuffer()
	{
		auto buf = std::make_shared<D3D11VertexBuffer>();
		buf->Register();
		return buf;
	}

	Ptr<VertexInputLayout> D3D11RenderFactory::CreateVertexInputLayout()
	{
		auto vi = std::make_shared<D3D11VertexInputLayout>();
		vi->Register();
		return vi;
	}

	Ptr<TransientBuffer> D3D11RenderFactory::CreateTransientBuffer()
	{
		return std::make_shared<D3D11TransientBuffer>();
	}

	Ptr<BlendState> D3D11RenderFactory::CreateBlendState()
	{
		auto state = std::make_shared<D3D11BlendState>();
		state->Register();
		return state;
	}

	Ptr<DepthStencilState> D3D11RenderFactory::CreateDepthStencilState()
	{
		auto state = std::make_shared<D3D11DepthStencilState>();
		state->Register();
		return state;
	}

	Ptr<RasterizerState> D3D11RenderFactory::CreateRasterizerState()
	{
		auto state = std::make_shared<D3D11RasterizerState>();
		state->Register();
		return state;
	}

	Ptr<Sampler> D3D11RenderFactory::CreateSampler()
	{
		auto sampler = std::make_shared<D3D11Sampler>();
		sampler->Register();
		return sampler;
	}


	String GetTargetName(ShaderType shaderType, ShaderModel shaderModel)
	{
		static std::map<ShaderType, std::map<ShaderModel, String>> shaderTargetNameMap =
		{
			{ SHADER_VS, { { SM_4, "vs_4_0" }, { SM_5, "vs_5_0" } } },
			{ SHADER_PS, { { SM_4, "ps_4_0" }, { SM_5, "ps_5_0" } } },
			{ SHADER_GS, { { SM_4, "gs_4_0" }, { SM_5, "gs_5_0" } } },
			{ SHADER_DS, { { SM_4, "" },	   { SM_5, "ds_5_0" } } },
			{ SHADER_HS, { { SM_4, "" },       { SM_5, "hs_5_0" } } },
			{ SHADER_CS, { { SM_4, "" },       { SM_5, "cs_5_0" } } }
		};

		return shaderTargetNameMap[shaderType][shaderModel];
	}

	bool D3D11RenderFactory::CompileShader(
		ShaderType shaderType,
		const String & shaderCode,
		const String & entryName,
		std::shared_ptr<uint8_t> & outCompiledData,
		int32_t & outDataSize)
	{
		ID3DBlob *pCode = nullptr;
		ID3DBlob *pError = nullptr;

		uint32_t flag = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#if !(defined (DEBUG) || defined (_DEBUG))
		//flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
		flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		_compileFunc(
			shaderCode.c_str(), //code
			shaderCode.size(), //code size
			entryName.c_str(), // source name
			0, // defines
			0, // include
			entryName.c_str(), // entry point
			GetTargetName(shaderType, RenderEngine::GetShaderModel()).c_str(), // target name
			flag, // flag1
			0, // flag2
			&pCode, // out compiled code
			&pError // out error
			);

		if (nullptr != pError)
		{
			ToyGE_LOG(LT_WARNING, "Shader compile error!");
			ToyGE_LOG(LT_RAW, reinterpret_cast<char*>(pError->GetBufferPointer()));
			ToyGE_LOG(LT_RAW, shaderCode.c_str());

			if(pCode)
				pCode->Release();
			if (pError)
				pError->Release();

			return false;
		}

		outCompiledData = MakeBufferedDataShared(pCode->GetBufferSize());
		memcpy(outCompiledData.get(), pCode->GetBufferPointer(), pCode->GetBufferSize());
		outDataSize = static_cast<int32_t>(pCode->GetBufferSize());

		if (pCode)
			pCode->Release();
		if (pError)
			pError->Release();

		return true;
	}

	bool D3D11RenderFactory::GetShaderProgramResourceMap(
		const std::shared_ptr<const uint8_t> & compiledData,
		int32_t dataSize,
		ShaderProgramResourceMap & outResourceMap)
	{
		ID3D11ShaderReflection* pReflector = nullptr;
		_reflectFunc(compiledData.get(), dataSize, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&pReflector));
		if (nullptr == pReflector)
		{
			ToyGE_LOG(LT_WARNING, "Cannot get shader info!");
			return false;
		}

		D3D11_SHADER_DESC d3dShaderDesc;
		pReflector->GetDesc(&d3dShaderDesc);

		outResourceMap.constantBuffers.clear();
		outResourceMap.srvs.clear();
		outResourceMap.uavs.clear();
		outResourceMap.samplers.clear();

		// CBs
		for (uint32_t cbIndex = 0; cbIndex != d3dShaderDesc.ConstantBuffers; ++cbIndex)
		{
			auto d3dCB = pReflector->GetConstantBufferByIndex(cbIndex);
			D3D11_SHADER_BUFFER_DESC d3dCBDesc;
			d3dCB->GetDesc(&d3dCBDesc);
			if (d3dCBDesc.Type != D3D_CT_CBUFFER)
				continue;

			BoundConstantBuffer cbDesc;
			cbDesc.name = d3dCBDesc.Name;
			cbDesc.bytesSize = d3dCBDesc.Size;

			// Variables
			for (uint32_t varIndex = 0; varIndex != d3dCBDesc.Variables; ++varIndex)
			{
				D3D11_SHADER_VARIABLE_DESC d3dCBVarDesc;
				d3dCB->GetVariableByIndex(varIndex)->GetDesc(&d3dCBVarDesc);
				BoundCBVariable cbVarDesc;
				cbVarDesc.name = d3dCBVarDesc.Name;
				cbVarDesc.bytesOffset = d3dCBVarDesc.StartOffset;
				cbVarDesc.bytesSize = d3dCBVarDesc.Size;

				cbDesc.variables.push_back(cbVarDesc);
			}

			outResourceMap.constantBuffers.push_back(cbDesc);
		}

		// Bound resources
		for (uint32_t resIndex = 0; resIndex != d3dShaderDesc.BoundResources; ++resIndex)
		{
			D3D11_SHADER_INPUT_BIND_DESC d3dResDesc;
			pReflector->GetResourceBindingDesc(resIndex, &d3dResDesc);

			BoundResource resDesc;
			resDesc.name = d3dResDesc.Name;
			resDesc.boundIndex = d3dResDesc.BindPoint;

			switch (d3dResDesc.Type)
			{
			case D3D_SIT_CBUFFER:
			case D3D_SIT_TBUFFER:
				for (auto & cbDesc : outResourceMap.constantBuffers)
					if (cbDesc.name == resDesc.name)
						cbDesc.boundIndex = resDesc.boundIndex;
				break;

			case D3D_SIT_TEXTURE:
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_BYTEADDRESS:
				outResourceMap.srvs.push_back(resDesc);
				break;

			case D3D_SIT_UAV_RWTYPED:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				outResourceMap.uavs.push_back(resDesc);
				break;

			case D3D_SIT_SAMPLER:
				outResourceMap.samplers.push_back(resDesc);
				break;

			default:
				break;
			}
		}

		return true;
	}
}