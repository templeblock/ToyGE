#pragma once
#ifndef D3D11SHADERPROGRAM_H
#define D3D11SHADERPROGRAM_H

#include "ToyGE\RenderEngine\ShaderProgram.h"
#include "ToyGE\D3D11\D3D11PreInclude.h"

namespace ToyGE
{
	class D3D11ShaderProgram : public ShaderProgram
	{
	public:
		D3D11ShaderProgram(ShaderType type)
			: ShaderProgram(type)
		{

		}

		virtual ~D3D11ShaderProgram() = default;
	};


	class D3D11VertexShaderProgram : public D3D11ShaderProgram
	{
	public:
		D3D11VertexShaderProgram();

		virtual void Init() override;

		virtual void Release() override
		{
			D3D11ShaderProgram::Release();

			_hardwareVertexShader = nullptr;
		}

		const Ptr<ID3D11VertexShader> & GetHardwareVertexShader()
		{
			return _hardwareVertexShader;
		}

		uint32_t VertexShaderProgramID() const
		{
			return _id;
		}

	private:
		static uint32_t _gVertexShaderProgramID;
		uint32_t _id;

		Ptr<ID3D11VertexShader> _hardwareVertexShader;
	};


	class D3D11HullShaderProgram : public D3D11ShaderProgram
	{
	public:
		D3D11HullShaderProgram()
			: D3D11ShaderProgram(SHADER_HS)
		{

		}

		virtual void Init() override;

		virtual void Release() override
		{
			D3D11ShaderProgram::Release();

			_hardwareHullShader = nullptr;
		}

		const Ptr<ID3D11HullShader> & GetHardwareHullShader()
		{
			return _hardwareHullShader;
		}

	private:
		Ptr<ID3D11HullShader> _hardwareHullShader;
	};


	class D3D11DomainShaderProgram : public D3D11ShaderProgram
	{
	public:
		D3D11DomainShaderProgram()
			: D3D11ShaderProgram(SHADER_DS)
		{

		}

		virtual void Init() override;

		virtual void Release() override
		{
			D3D11ShaderProgram::Release();

			_hardwareDomainShader = nullptr;
		}

		const Ptr<ID3D11DomainShader> & GetHardwareDomainShader()
		{
			return _hardwareDomainShader;
		}

	private:
		Ptr<ID3D11DomainShader> _hardwareDomainShader;
	};


	class D3D11GeometryShaderProgram : public D3D11ShaderProgram
	{
	public:
		D3D11GeometryShaderProgram()
			: D3D11ShaderProgram(SHADER_GS)
		{

		}

		virtual void Init() override;

		virtual void Release() override
		{
			D3D11ShaderProgram::Release();

			_hardwareGeometryShader = nullptr;
		}

		const Ptr<ID3D11GeometryShader> & GetHardwareGeometryShader()
		{
			return _hardwareGeometryShader;
		}

	private:
		Ptr<ID3D11GeometryShader> _hardwareGeometryShader;
	};


	class D3D11PixelShaderProgram : public D3D11ShaderProgram
	{
	public:
		D3D11PixelShaderProgram()
			: D3D11ShaderProgram(SHADER_PS)
		{

		}

		virtual void Init() override;

		virtual void Release() override
		{
			D3D11ShaderProgram::Release();

			_hardwarePixelShader = nullptr;
		}

		const Ptr<ID3D11PixelShader> & GetHardwarePixelShader()
		{
			return _hardwarePixelShader;
		}

	private:
		Ptr<ID3D11PixelShader> _hardwarePixelShader;
	};


	class D3D11ComputeShaderProgram : public D3D11ShaderProgram
	{
	public:
		D3D11ComputeShaderProgram()
			: D3D11ShaderProgram(SHADER_CS)
		{

		}

		virtual void Init() override;

		virtual void Release() override
		{
			D3D11ShaderProgram::Release();

			_hardwareComputeShader = nullptr;
		}

		const Ptr<ID3D11ComputeShader> & GetHardwareComputeShader()
		{
			return _hardwareComputeShader;
		}

	private:
		Ptr<ID3D11ComputeShader> _hardwareComputeShader;
	};
}

#endif