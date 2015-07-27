#pragma once
#ifndef D3D11RENDERCONTEXT_H
#define D3D11RENDERCONTEXT_H

#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class Window;

	class D3D11RenderContext : public RenderContext
	{
		typedef void(_stdcall SetSRVFunc)(ID3D11DeviceContext *, uint32_t offset, uint32_t numViews, ID3D11ShaderResourceView * const *);
		typedef void(_stdcall SetUAVFunc)(ID3D11DeviceContext *, uint32_t offset, uint32_t numViews, ID3D11UnorderedAccessView * const *, const uint32_t *);
		typedef void(_stdcall SetBufferFunc)(ID3D11DeviceContext *, uint32_t offset, uint32_t numBuffers, ID3D11Buffer * const *);
		typedef void(_stdcall SetSamplerFunc)(ID3D11DeviceContext *, uint32_t offset, uint32_t numSamplers, ID3D11SamplerState * const *);

	public:
		D3D11RenderContext(const Ptr<Window> & window, const Ptr<ID3D11DeviceContext> & deviceContext);

		void ResetRenderTargetAndDepthStencil();

	private:
		Ptr<Window> _window;
		Ptr<ID3D11DeviceContext> _rawD3DDeviceContext;

		int32_t _cachedNumVBs;

		static std::map<ShaderType, std::function<SetSRVFunc>> _setSRVFuncMap;
		static std::map<ShaderType, std::function<SetUAVFunc>> _setUAVFuncMap;
		static std::map<ShaderType, std::function<SetBufferFunc>> _setBufferFuncMap;
		static std::map<ShaderType, std::function<SetSamplerFunc>> _setSamplerFuncMap;

		void DoClearRenderTargets(const std::vector<ResourceView> & renderTargets, const float4 & color) override;

		void DoClearDepthStencil(const ResourceView & depthStencil, float depth, uint8_t stencil) override;

		void DoSetViewport(const RenderViewport & viewport) override;

		void DoSetRenderTargetsAndDepthStencil(const std::vector<ResourceView> & targets, const ResourceView & depthStencil) override;

		void DoSetRenderInput(const Ptr<RenderInput> & input) override;

		void DoSetShaderProgram(const Ptr<ShaderProgram> & program) override;

		void DoResetShaderProgram(ShaderType shaderType) override;

		void DoSetShaderResources(ShaderType shaderType, const std::vector<ResourceView> & resources, int32_t offset) override;

		void DoSetRTsAndUAVs(const std::vector<ResourceView> & targets, const ResourceView & depthStencil, const std::vector<ResourceView> & uavs) override;

		void DoSetUAVs(ShaderType shaderType, const std::vector<ResourceView> & resources, int32_t offset) override;

		void DoSetShaderBuffers(ShaderType shaderType, const std::vector<Ptr<RenderBuffer>> & buffers, int32_t offset) override;

		void DoSetShaderSamplers(ShaderType shaderType, const std::vector<Ptr<Sampler>> & samplers, int32_t offset) override;

		void DoSetBlendState(const Ptr<BlendState> & state, const std::vector<float> & blendFactors, uint32_t sampleMask) override;

		void DoSetDepthStencilState(const Ptr<DepthStencilState> & state, uint32_t steincilRef = 0) override;

		void DoSetRasterizerState(const Ptr<RasterizerState> & state) override;

		void DoDrawVertices(int32_t numVertices, int32_t vertexStart) override;

		void DoDrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase) override;

		void DoDrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset) override;

		void DoCompute(int32_t groupX, int32_t groupY, int32_t groupZ) override;


		void GetRawD3DRenderTargets(const std::vector<ResourceView> & renderTargets, std::vector<ID3D11RenderTargetView*> & outRenderTargets);

		ID3D11DepthStencilView * GetRawD3DDepthStencil(const ResourceView & depthStencil);
	};
}

#endif