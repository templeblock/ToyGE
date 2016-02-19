#pragma once
#ifndef D3D11RENDERCONTEXT_H
#define D3D11RENDERCONTEXT_H

#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\D3D11\D3D11PreInclude.h"

namespace ToyGE
{
	class D3D11RenderContext : public RenderContext
	{
	public:

	private:
		virtual void DoSetViewport(const RenderViewport & viewport) override;

		virtual void DoSetVertexBuffer(const std::vector<Ptr<VertexBuffer>> & vertexBuffers, const std::vector<int32_t> & byteOffsets) override;

		virtual void DoSetIndexBuffer(const Ptr<RenderBuffer> & indexBuffer, int32_t bytesOffset) override;

		virtual void DoSetVertexInputLayout(const Ptr<VertexInputLayout> & vertexInputLayout) override;

		virtual void DoSetPrimitiveTopology(PrimitiveTopology primitiveTopology) override;

		virtual void DoSetRenderTargetsAndDepthStencil(const std::vector<Ptr<RenderTargetView>> & renderTargets, const Ptr<DepthStencilView> & depthStencil) override;

		virtual void DoSetShaderProgram(ShaderType shaderType, const Ptr<ShaderProgram> & program) override;

		virtual void DoSetSRVs(ShaderType shaderType, const std::vector<Ptr<ShaderResourceView>> & srvs, int32_t offset) override;

		virtual void DoSetUAVs(ShaderType shaderType, const std::vector<Ptr<UnorderedAccessView>> & uavs, const std::vector<int32_t> & uavInitialCounts, int32_t offset) override;

		virtual void DoSetSamplers(ShaderType shaderType, const std::vector<Ptr<Sampler>> & samplers, int32_t offset) override;

		virtual void DoSetRTVsAndUAVs(const std::vector<Ptr<RenderTargetView>> & renderTargets, const Ptr<DepthStencilView> & depthStencil, const std::vector<Ptr<UnorderedAccessView>> & uavs, const std::vector<int32_t> & uavInitialCounts) override;

		virtual void DoSetCBs(ShaderType shaderType, const std::vector<Ptr<RenderBuffer>> & buffers, int32_t offset) override;

		virtual void DoSetBlendState(const Ptr<BlendState> & state, const float4 & blendFactors, uint32_t sampleMask) override;

		virtual void DoSetDepthStencilState(const Ptr<DepthStencilState> & state, uint32_t steincilRef = 0) override;

		virtual void DoSetRasterizerState(const Ptr<RasterizerState> & state) override;

		virtual void DoDrawVertices(int32_t numVertices, int32_t vertexStart) override;

		virtual void DoDrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase) override;

		virtual void DoDrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset) override;

		virtual void DoCompute(int32_t groupX, int32_t groupY, int32_t groupZ) override;

		virtual void DoClearRenderTarget(const Ptr<RenderTargetView> & renderTarget, const float4 & color) override;

		virtual void DoClearDepthStencil(const Ptr<DepthStencilView> & depthStencil, float depth, uint8_t stencil) override;
	};
}

#endif