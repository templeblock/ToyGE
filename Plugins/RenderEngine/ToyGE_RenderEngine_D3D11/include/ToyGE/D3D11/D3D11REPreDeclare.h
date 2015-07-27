#pragma once
#ifndef D3D11REPREDECLARE_H
#define D3D11REPREDECLARE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include <d3d11_1.h>
#include <d3dcompiler.h>

#ifdef TOYGE_D3D11RE_SOURCE
#define TOYGE_D3D11RE_API _declspec(dllexport)
#else
#define TOYGE_D3D11RE_API _declspec(dllimport)
#endif

namespace ToyGE
{
	using D3DCompileFunc = decltype(::D3DCompile) *;
	using D3DReflectFunc = decltype(::D3DReflect) *;

	//class D3D11RenderContext;
	//using Ptr<D3D11RenderContext> = std::shared_ptr < D3D11RenderContext > ;
	//class D3D11RenderEngine;
	//using Ptr<D3D11RenderEngine> = std::shared_ptr < D3D11RenderEngine > ;
	//class D3D11RenderFactory;
	//using Ptr<D3D11RenderFactory> = std::shared_ptr < D3D11RenderFactory > ;
	//class D3D11ShaderProgram;
	//using Ptr<D3D11ShaderProgram> = std::shared_ptr < D3D11ShaderProgram >;
	//class D3D11VertexShaderProgram;
	//using Ptr<D3D11VertexShaderProgram> = std::shared_ptr < D3D11VertexShaderProgram >;
	//class D3D11HullShaderProgram;
	//using D3D11HullPtr<ShaderProgram> = std::shared_ptr < D3D11HullShaderProgram >;
	//class D3D11DomainShaderProgram;
	//using D3D11DomainPtr<ShaderProgram> = std::shared_ptr < D3D11DomainShaderProgram >;
	//class D3D11GeometryShaderProgram;
	//using D3D11GeometryPtr<ShaderProgram> = std::shared_ptr < D3D11GeometryShaderProgram >;
	//class D3D11PixelShaderProgram;
	//using D3D11PixelPtr<ShaderProgram> = std::shared_ptr < D3D11PixelShaderProgram >;
	//class D3D11ComputeShaderProgram;
	//using D3D11ComputePtr<ShaderProgram> = std::shared_ptr < D3D11ComputeShaderProgram >;
	//class D3D11Texture;
	//using Ptr<D3D11Texture> = std::shared_ptr < D3D11Texture > ;
	//class D3D11Texture1D;
	//using Ptr<D3D11Texture1D> = std::shared_ptr < D3D11Texture1D >;
	//class D3D11Texture2D;
	//using Ptr<D3D11Texture2D> = std::shared_ptr < D3D11Texture2D >;
	//class D3D11Texture3D;
	//using Ptr<D3D11Texture3D> = std::shared_ptr < D3D11Texture3D >;
	//class D3D11TextureCube;
	//using Ptr<D3D11TextureCube> = std::shared_ptr < D3D11TextureCube >;
	//class D3D11RenderBuffer;
	//using Ptr<D3D11RenderBuffer> = std::shared_ptr < D3D11RenderBuffer > ;
	//class D3D11BlendState;
	//using Ptr<D3D11BlendState> = std::shared_ptr < D3D11BlendState >;
	//class D3D11DepthStencilState;
	//using Ptr<D3D11DepthStencilState> = std::shared_ptr < D3D11DepthStencilState >;
	//class D3D11RasterizerState;
	//using Ptr<D3D11RasterizerState> = std::shared_ptr < D3D11RasterizerState >;
	//class D3D11Sampler;
	//using Ptr<D3D11Sampler> = std::shared_ptr < D3D11Sampler >;
	//class D3D11RenderInput;
	//using Ptr<D3D11RenderInput> = std::shared_ptr < D3D11RenderInput > ;

	//using Ptr<ID3D11Device> = std::shared_ptr < ID3D11Device > ;
	//using Ptr<ID3D11DeviceContext> = std::shared_ptr < ID3D11DeviceContext > ;
	//using Ptr<IDXGIFactory1> = std::shared_ptr < IDXGIFactory1 > ;
	//using Ptr<IDXGIAdapter1> = std::shared_ptr < IDXGIAdapter1 > ;
	//using Ptr<IDXGISwapChain> = std::shared_ptr < IDXGISwapChain > ;
	//using Ptr<ID3D11Resource> = std::shared_ptr < ID3D11Resource > ;
	//using Ptr<ID3D11Texture1D> = std::shared_ptr < ID3D11Texture1D > ;
	//using Ptr<ID3D11Texture2D> = std::shared_ptr < ID3D11Texture2D > ;
	//using Ptr<ID3D11Texture3D> = std::shared_ptr < ID3D11Texture3D > ;
	//using Ptr<ID3D11ShaderResourceView> = std::shared_ptr < ID3D11ShaderResourceView > ;
	//using Ptr<ID3D11RenderTargetView> = std::shared_ptr < ID3D11RenderTargetView > ;
	//using Ptr<ID3D11DepthStencilView> = std::shared_ptr < ID3D11DepthStencilView > ;
	//using Ptr<ID3D11UnorderedAccessView> = std::shared_ptr < ID3D11UnorderedAccessView > ;
	//using Ptr<ID3D11VertexShader> = std::shared_ptr < ID3D11VertexShader > ;
	//using Ptr<ID3D11HullShader> = std::shared_ptr < ID3D11HullShader > ;
	//using Ptr<ID3D11DomainShader> = std::shared_ptr < ID3D11DomainShader > ;
	//using Ptr<ID3D11GeometryShader> = std::shared_ptr < ID3D11GeometryShader > ;
	//using Ptr<ID3D11PixelShader> = std::shared_ptr < ID3D11PixelShader > ;
	//using Ptr<ID3D11ComputeShader> = std::shared_ptr < ID3D11ComputeShader > ;
	//using Ptr<ID3D11InputLayout> = std::shared_ptr < ID3D11InputLayout > ;
	//using ID3DBlobPtr = std::shared_ptr < ID3DBlob > ;
	//using Ptr<ID3D11Buffer> = std::shared_ptr < ID3D11Buffer > ;
	//using Ptr<ID3D11BlendState> = std::shared_ptr < ID3D11BlendState > ;
	//using Ptr<ID3D11DepthStencilState> = std::shared_ptr < ID3D11DepthStencilState > ;
	//using Ptr<ID3D11RasterizerState> = std::shared_ptr < ID3D11RasterizerState > ;
	//using Ptr<ID3D11SamplerState> = std::shared_ptr < ID3D11SamplerState > ;
	//using ID3D11DebugPtr = std::shared_ptr < ID3D11Debug > ;
	//using Ptr<ID3D11Query> = std::shared_ptr < ID3D11Query > ;
}

#endif