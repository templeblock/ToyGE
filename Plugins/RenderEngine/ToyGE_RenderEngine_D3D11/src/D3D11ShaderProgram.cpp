#include "ToyGE\D3D11\D3D11ShaderProgram.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Logger.h"

namespace ToyGE
{
//	D3D11ShaderProgram::D3D11ShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName)
//		: ShaderProgram(type, pData, dataSize, entryName)
//	{
//		D3D_SHADER_MACRO *pD3DMacros = nullptr;
//
//		ID3DBlob *pCode = nullptr;
//		ID3DBlob *pError = nullptr;
//
//		uint32_t flag = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
//#if !(defined (DEBUG) || defined (_DEBUG))
//		//flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
//		flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
//#else
//		//flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
//#endif
//
//		compileFunc(pData, dataSize, 0, pD3DMacros, 0,
//			entryName.c_str(), GetTargetName().c_str(), flag, 0, &pCode, &pError);
//		/*::D3DCompile(pData, dataSize, 0, pD3DMacros, 0,
//			entryName.c_str(), GetTargetName().c_str(), flag, 0, &pCode, &pError);*/
//
//		if (nullptr != pError)
//		{
//			Logger::Log(reinterpret_cast<char*>(pError->GetBufferPointer()));
//
//			std::stringstream ss;
//			auto str = reinterpret_cast<const char*>(pData);
//			for (size_t i = 0; str[i] != 0; ++i)
//			{
//				if (str[i] == '%')
//					ss << '%';
//
//				ss << str[i];
//			}
//			
//			Logger::Log(ss.str().c_str());
//			ToyGE_ASSERT_FAIL("Compile error!");
//		}
//
//		_d3dCompiledCode = MakeComShared(pCode);
//		_compiledCode.bufferPointer = _d3dCompiledCode->GetBufferPointer();
//		_compiledCode.bufferSize = _d3dCompiledCode->GetBufferSize();
//	}
//
//	D3D11ShaderProgram::D3D11ShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize)
//		: ShaderProgram(type, pData, dataSize),
//		_preCompiledData(pData)
//	{
//		_compiledCode.bufferPointer = _preCompiledData.get();
//		_compiledCode.bufferSize = dataSize;
//	}
//
//	D3D11ShaderProgram::D3D11ShaderProgram(ShaderType type, const std::shared_ptr<uint8_t> & compiledData, size_t dataSize)
//		: ShaderProgram(type, compiledData, dataSize)
//	{
//
//	}

	void D3D11ShaderProgram::Init(const std::shared_ptr<uint8_t> & compiledData, size_t dataSize)
	{
		_compiledData = compiledData;
		_compiledDataSize = dataSize;

		DoInitShaderProgram();
	}

	/*String D3D11ShaderProgram::GetTargetName()
	{
		String tagetName;
		switch (_type)
		{
		case SHADER_VS:
			tagetName = "vs_5_0";
			break;
		case SHADER_HS:
			tagetName = "hs_5_0";
			break;
		case SHADER_DS:
			tagetName = "ds_5_0";
			break;
		case SHADER_GS:
			tagetName = "gs_5_0";
			break;
		case SHADER_PS:
			tagetName = "ps_5_0";
			break;
		case SHADER_CS:
			tagetName = "cs_5_0";
			break;
		default:
			break;
		}

		return tagetName;
	}*/


	uint32_t D3D11VertexShaderProgram::_gVertexShaderProgramID = 0;

	//D3D11VertexShaderProgram::D3D11VertexShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName)
	//	: D3D11ShaderProgram(compileFunc, type, pData, dataSize, entryName)
	//{
	//	DoCreateShaderProgram();

	//	_id = _gVertexShaderProgramID ++;
	//}

	//D3D11VertexShaderProgram::D3D11VertexShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize)
	//	: D3D11ShaderProgram(compileFunc, type, pData, dataSize)
	//{
	//	DoCreateShaderProgram();

	//	_id = _gVertexShaderProgramID++;
	//}

	D3D11VertexShaderProgram::D3D11VertexShaderProgram()
		: D3D11ShaderProgram(SHADER_VS)
	{
		_id = _gVertexShaderProgramID++;
	}

	void D3D11VertexShaderProgram::DoInitShaderProgram()
	{
		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>( Global::GetRenderEngine().get() );
		ID3D11VertexShader *pShaderProgram = nullptr;
		pD3DEngine->RawD3DDevice()->CreateVertexShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_rawD3DVertexShader = MakeComShared(pShaderProgram);
	}


	void D3D11HullShaderProgram::DoInitShaderProgram()
	{
		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11HullShader *pShaderProgram = nullptr;
		pD3DEngine->RawD3DDevice()->CreateHullShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_rawD3DHullShader = MakeComShared(pShaderProgram);
	}


	void D3D11DomainShaderProgram::DoInitShaderProgram()
	{
		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11DomainShader *pShaderProgram = nullptr;
		pD3DEngine->RawD3DDevice()->CreateDomainShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_rawD3DDomainShader = MakeComShared(pShaderProgram);
	}


	void D3D11GeometryShaderProgram::DoInitShaderProgram()
	{
		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11GeometryShader *pShaderProgram = nullptr;
		pD3DEngine->RawD3DDevice()->CreateGeometryShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_rawD3DGeometryShader = MakeComShared(pShaderProgram);
	}


	void D3D11PixelShaderProgram::DoInitShaderProgram()
	{
		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11PixelShader *pShaderProgram = nullptr;
		pD3DEngine->RawD3DDevice()->CreatePixelShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_rawD3DPixelShader = MakeComShared(pShaderProgram);
	}


	void D3D11ComputeShaderProgram::DoInitShaderProgram()
	{
		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11ComputeShader *pShaderProgram = nullptr;
		pD3DEngine->RawD3DDevice()->CreateComputeShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_rawD3DComputeShader = MakeComShared(pShaderProgram);
	}
}