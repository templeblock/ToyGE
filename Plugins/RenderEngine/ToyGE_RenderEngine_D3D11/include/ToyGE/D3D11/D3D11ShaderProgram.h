#pragma once
#ifndef D3D11SHADERPROGRAM_H
#define D3D11SHADERPROGRAM_H

#include "ToyGE\RenderEngine\ShaderProgram.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class D3D11ShaderProgram : public ShaderProgram
	{
	public:
		/*D3D11ShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName);
		D3D11ShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize);*/
		D3D11ShaderProgram(ShaderType type)
			: ShaderProgram(type)
		{

		}

		void Init(const std::shared_ptr<uint8_t> & compiledData, size_t dataSize);

		virtual ~D3D11ShaderProgram() = default;

		//virtual String GetTargetName() = 0;

	protected:
		//ID3DBlobPtr _d3dCompiledCode;
		//std::shared_ptr<uint8_t> _preCompiledData;

		virtual void DoLoad(const Ptr<Reader> & reader) override
		{
			ShaderProgram::DoLoad(reader);
			Init(_compiledData, _compiledDataSize);
		}

		virtual void DoInitShaderProgram() = 0;

	/*private:
		String GetTargetName();*/
	};


	class D3D11VertexShaderProgram : public D3D11ShaderProgram
	{
	public:
		//D3D11VertexShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName);
		//D3D11VertexShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize);

		D3D11VertexShaderProgram();

		const Ptr<ID3D11VertexShader> & RawD3DVertexShader()
		{
			return _rawD3DVertexShader;
		}

		uint32_t VertexShaderProgramID() const
		{
			return _id;
		}

		//String GetTargetName() override
		//{
		//	return "vs_5_0";
		//}

	protected:
		void DoInitShaderProgram() override;


	private:
		static uint32_t _gVertexShaderProgramID;
		uint32_t _id;

		Ptr<ID3D11VertexShader> _rawD3DVertexShader;
	};


	class D3D11HullShaderProgram : public D3D11ShaderProgram
	{
	public:
		//D3D11HullShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName);
		//D3D11HullShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize);
		D3D11HullShaderProgram()
			: D3D11ShaderProgram(SHADER_HS)
		{

		}

		const Ptr<ID3D11HullShader> & RawD3DHullShader()
		{
			return _rawD3DHullShader;
		}

		//String GetTargetName() override
		//{
		//	return "hs_5_0";
		//}

	protected:
		void DoInitShaderProgram() override;

	private:
		Ptr<ID3D11HullShader> _rawD3DHullShader;
	};


	class D3D11DomainShaderProgram : public D3D11ShaderProgram
	{
	public:
		//D3D11DomainShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName);
		//D3D11DomainShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize);

		D3D11DomainShaderProgram()
			: D3D11ShaderProgram(SHADER_DS)
		{

		}

		const Ptr<ID3D11DomainShader> & RawD3DDomainShader()
		{
			return _rawD3DDomainShader;
		}

		//String GetTargetName() override
		//{
		//	return "ds_5_0";
		//}

	protected:
		void DoInitShaderProgram() override;

	private:
		Ptr<ID3D11DomainShader> _rawD3DDomainShader;
	};


	class D3D11GeometryShaderProgram : public D3D11ShaderProgram
	{
	public:
		//D3D11GeometryShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName);
		//D3D11GeometryShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize);

		D3D11GeometryShaderProgram()
			: D3D11ShaderProgram(SHADER_GS)
		{

		}

		const Ptr<ID3D11GeometryShader> & RawD3DGeometryShader()
		{
			return _rawD3DGeometryShader;
		}

		//String GetTargetName() override
		//{
		//	return "gs_5_0";
		//}

	protected:
		void DoInitShaderProgram() override;

	private:
		Ptr<ID3D11GeometryShader> _rawD3DGeometryShader;
	};


	class D3D11PixelShaderProgram : public D3D11ShaderProgram
	{
	public:
		//D3D11PixelShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName);
		//D3D11PixelShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize);

		D3D11PixelShaderProgram()
			: D3D11ShaderProgram(SHADER_PS)
		{

		}

		const Ptr<ID3D11PixelShader> & RawD3DPixelShader()
		{
			return _rawD3DPixelShader;
		}

		//String GetTargetName() override
		//{
		//	return "ps_5_0";
		//}

	protected:
		void DoInitShaderProgram() override;

	private:
		Ptr<ID3D11PixelShader> _rawD3DPixelShader;
	};


	class D3D11ComputeShaderProgram : public D3D11ShaderProgram
	{
	public:
		//D3D11ComputeShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const void *pData, size_t dataSize, const String & entryName);
		//D3D11ComputeShaderProgram(D3DCompileFunc compileFunc, ShaderType type, const std::shared_ptr<uint8_t> & pData, size_t dataSize);

		D3D11ComputeShaderProgram()
			: D3D11ShaderProgram(SHADER_CS)
		{

		}

		const Ptr<ID3D11ComputeShader> & RawD3DComputeShader()
		{
			return _rawD3DComputeShader;
		}

		//String GetTargetName() override
		//{
		//	return "cs_5_0";
		//}

	protected:
		void DoInitShaderProgram() override;

	private:
		Ptr<ID3D11ComputeShader> _rawD3DComputeShader;
	};
}

#endif