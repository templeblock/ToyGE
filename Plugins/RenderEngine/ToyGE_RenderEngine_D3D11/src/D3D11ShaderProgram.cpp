#include "ToyGE\D3D11\D3D11ShaderProgram.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\D3D11\D3D11Util.h"

namespace ToyGE
{

	uint32_t D3D11VertexShaderProgram::_gVertexShaderProgramID = 0;

	D3D11VertexShaderProgram::D3D11VertexShaderProgram()
		: D3D11ShaderProgram(SHADER_VS)
	{
		_id = _gVertexShaderProgramID++;
	}

	void D3D11VertexShaderProgram::Init()
	{
		D3D11ShaderProgram::Init();

		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>( Global::GetRenderEngine().get() );
		ID3D11VertexShader *pShaderProgram = nullptr;
		D3D11RenderEngine::d3d11Device->CreateVertexShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_hardwareVertexShader = MakeComShared(pShaderProgram);
	}


	void D3D11HullShaderProgram::Init()
	{
		D3D11ShaderProgram::Init();

		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11HullShader *pShaderProgram = nullptr;
		D3D11RenderEngine::d3d11Device->CreateHullShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_hardwareHullShader = MakeComShared(pShaderProgram);
	}


	void D3D11DomainShaderProgram::Init()
	{
		D3D11ShaderProgram::Init();

		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11DomainShader *pShaderProgram = nullptr;
		D3D11RenderEngine::d3d11Device->CreateDomainShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_hardwareDomainShader = MakeComShared(pShaderProgram);
	}


	void D3D11GeometryShaderProgram::Init()
	{
		D3D11ShaderProgram::Init();

		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11GeometryShader *pShaderProgram = nullptr;
		D3D11RenderEngine::d3d11Device->CreateGeometryShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_hardwareGeometryShader = MakeComShared(pShaderProgram);
	}


	void D3D11PixelShaderProgram::Init()
	{
		D3D11ShaderProgram::Init();

		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11PixelShader *pShaderProgram = nullptr;
		D3D11RenderEngine::d3d11Device->CreatePixelShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_hardwarePixelShader = MakeComShared(pShaderProgram);
	}


	void D3D11ComputeShaderProgram::Init()
	{
		D3D11ShaderProgram::Init();

		D3D11RenderEngine *pD3DEngine = static_cast<D3D11RenderEngine*>(Global::GetRenderEngine().get());
		ID3D11ComputeShader *pShaderProgram = nullptr;
		D3D11RenderEngine::d3d11Device->CreateComputeShader(_compiledData.get(), _compiledDataSize, 0, &pShaderProgram);
		_hardwareComputeShader = MakeComShared(pShaderProgram);
	}
}