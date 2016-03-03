#pragma once
#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\RenderViewport.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class RenderTargetView;
	class DepthStencilView;
	class ShaderProgram;
	class ShaderResourceView;
	class UnorderedAccessView;
	class Sampler;
	class RenderBuffer;
	class BlendState;
	class DepthStencilState;
	class RasterizerState;
	class VertexBuffer;
	class VertexInputLayout;

	struct RenderContextState
	{
		// Viewport
		RenderViewport viewport;

		// Vertex buffer
		std::vector<Ptr<VertexBuffer>> vertexBuffers;
		std::vector<int32_t> vertexBufferOffsets;

		// Index buffer
		Ptr<RenderBuffer> indexBuffer;
		int32_t indexBufferOffset = 0;

		// Primitive topology
		PrimitiveTopology primitiveTopology;

		// Render targets
		std::vector<Ptr<RenderTargetView>> renderTargets;

		// Depth stencil
		Ptr<DepthStencilView> depthStencil;

		// Shaders
		std::array<Ptr<ShaderProgram>, ShaderTypeNum::NUM> shaders;

		// SRVs
		std::array<std::vector<Ptr<ShaderResourceView>>, ShaderTypeNum::NUM> srvs;

		// UAVs
		std::array<std::vector<Ptr<UnorderedAccessView>>, ShaderTypeNum::NUM> uavs;
		std::array<std::vector<int32_t>, ShaderTypeNum::NUM> uavInitialCounts;

		// Samplers
		std::array<std::vector<Ptr<Sampler>>, ShaderTypeNum::NUM> samplers;

		// CBs
		std::array<std::vector<Ptr<RenderBuffer>>, ShaderTypeNum::NUM> cbs;

		// Blend state
		Ptr<BlendState> blendState;
		float4 blendFactor;
		uint32_t blendSampleMask = 0xffffffff;

		// Depth stencil state
		Ptr<DepthStencilState> depthStencilState;
		uint32_t stencilRef = 0;

		// Rasterizer state
		Ptr<RasterizerState> rasterizerState;
	};

	class TOYGE_CORE_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		CLASS_SET(State, RenderContextState, _state);
		CLASS_GET(State, RenderContextState, _state);

		void SetViewport(const RenderViewport & viewport)
		{
			_state.viewport = viewport;
		}

		const RenderViewport & GetViewport()
		{
			return _state.viewport;
		}

		void SetVertexBuffer(const std::vector<Ptr<VertexBuffer>> & vertexBuffers, const std::vector<int32_t> & offsets = {})
		{
			_state.vertexBuffers = vertexBuffers;
			_state.vertexBufferOffsets = offsets;
			_state.vertexBufferOffsets.resize(vertexBuffers.size());
		}

		const std::vector<Ptr<VertexBuffer>> & GetVertexBuffer() const
		{
			return _state.vertexBuffers;
		}

		const std::vector<int32_t> & GetVertexBufferOffsets() const
		{
			return _state.vertexBufferOffsets;
		}

		void SetIndexBuffer(const Ptr<RenderBuffer> & indexBuffer, int32_t byteOffset = 0)
		{
			_state.indexBuffer = indexBuffer;
			_state.indexBufferOffset = byteOffset;
		}

		Ptr<RenderBuffer> GetIndexBuffer() const
		{
			return _state.indexBuffer;
		}

		int32_t GetIndexBufferOffset() const
		{
			return _state.indexBufferOffset;
		}

		void SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
		{
			_state.primitiveTopology = primitiveTopology;
		}

		PrimitiveTopology GetPrimitiveTopology() const
		{
			return _state.primitiveTopology;
		}

		void SetRenderTargets(const std::vector<Ptr<RenderTargetView>> & renderTargets);

		const std::vector<Ptr<RenderTargetView>> & GetRenderTargets() const
		{
			return _state.renderTargets;
		}

		void SetDepthStencil(const Ptr<DepthStencilView> & depthStencil);

		const Ptr<DepthStencilView> & GetDepthStencil() const
		{
			return _state.depthStencil;
		}

		void SetShader(const Ptr<ShaderProgram> & shader);

		void ResetShader(ShaderType shaderType);

		Ptr<ShaderProgram> GetShader(ShaderType shaderType) const;

		void SetSRVs(ShaderType shaderType, const std::vector<Ptr<ShaderResourceView>> & srvs);

		void SetUAVs(ShaderType shaderType, const std::vector<Ptr<UnorderedAccessView>> & uavs, const std::vector<int32_t> & uavInitialCounts);

		void SetSamplers(ShaderType shaderType, const std::vector<Ptr<Sampler>> & samplers);

		void SetCBs(ShaderType shaderType, const std::vector<Ptr<RenderBuffer>> & cbs);

		void SetBlendState(const Ptr<BlendState> & blendState, const float4 & blendFactors = 1.0f, uint32_t sampleMask = 0xffffffff);

		void SetDepthStencilState(const Ptr<DepthStencilState> & depthStencilState, uint32_t steincilRef = 0);

		void SetRasterizerState(const Ptr<RasterizerState> & rasterizerState);

		void DrawVertices(int32_t numVertices, int32_t vertexStart);

		void DrawVertices(int32_t vertexStart);

		void DrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase);

		void DrawIndexed(int32_t indexStart, int32_t indexBase);

		void DrawInstanced(int32_t numVerticesPerInstance, int32_t numInstances, int32_t vertexStart, int32_t instanceBase);

		void DrawIndexedInstanced(int32_t numIndicesPerInstance, int32_t numInstances, int32_t indexStart, int32_t indexBase, int32_t instanceBase);

		void DrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset);

		void Compute(int32_t groupX, int32_t groupY, int32_t groupZ);

		void ClearRenderTarget(const Ptr<RenderTargetView> & renderTarget, const float4 & color)
		{
			DoClearRenderTarget(renderTarget, color);
		}

		void ClearRenderTarget(const std::vector<Ptr<RenderTargetView>> & renderTargets, const float4 & color)
		{
			for (auto & rt : renderTargets)
				ClearRenderTarget(rt, color);
		}

		void ClearRenderTarget(const float4 & color)
		{
			ClearRenderTarget(_state.renderTargets, color);
		}

		void ClearDepthStencil(const Ptr<DepthStencilView> & depthStencil, float depth, uint8_t stencil)
		{
			DoClearDepthStencil(depthStencil, depth, stencil);
		}

		void ClearDepthStencil(float depth, uint8_t stencil)
		{
			ClearDepthStencil(_state.depthStencil, depth, stencil);
		}

	protected:
		RenderContextState _state;
		RenderContextState _stateCache;

		virtual void DoSetViewport(const RenderViewport & vp) = 0;

		virtual void DoSetVertexBuffer(const std::vector<Ptr<VertexBuffer>> & vertexBuffers, const std::vector<int32_t> & byteOffsets) = 0;

		virtual void DoSetIndexBuffer(const Ptr<RenderBuffer> & indexBuffer, int32_t byteOffset) = 0;

		virtual void DoSetVertexInputLayout(const Ptr<VertexInputLayout> & vertexInputLayout) = 0;

		virtual void DoSetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;

		virtual void DoSetRenderTargetsAndDepthStencil(const std::vector<Ptr<RenderTargetView>> & renderTargets, const Ptr<DepthStencilView> & depthStencil) = 0;

		virtual void DoSetShaderProgram(ShaderType shaderType, const Ptr<ShaderProgram> & program) = 0;

		virtual void DoSetSRVs(ShaderType shaderType, const std::vector<Ptr<ShaderResourceView>> & srvs, int32_t offset) = 0;

		virtual void DoSetUAVs(ShaderType shaderType, const std::vector<Ptr<UnorderedAccessView>> & uavs, const std::vector<int32_t> & uavInitialCounts, int32_t offset) = 0;

		virtual void DoSetSamplers(ShaderType shaderType, const std::vector<Ptr<Sampler>> & samplers, int32_t offset) = 0;

		virtual void DoSetRTVsAndUAVs(const std::vector<Ptr<RenderTargetView>> & renderTargets, const Ptr<DepthStencilView> & depthStencil, const std::vector<Ptr<UnorderedAccessView>> & uavs, const std::vector<int32_t> & uavInitialCounts) = 0;

		virtual void DoSetCBs(ShaderType shaderType, const std::vector<Ptr<RenderBuffer>> & buffers, int32_t offset) = 0;

		virtual void DoSetBlendState(const Ptr<BlendState> & state, const float4 & blendFactors, uint32_t sampleMask) = 0;

		virtual void DoSetDepthStencilState(const Ptr<DepthStencilState> & state, uint32_t steincilRef = 0) = 0;

		virtual void DoSetRasterizerState(const Ptr<RasterizerState> & state) = 0;

		virtual void DoDrawVertices(int32_t numVertices, int32_t vertexStart) = 0;

		virtual void DoDrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase) = 0;

		virtual void DoDrawInstanced(int32_t numVerticesPerInstance, int32_t numInstances, int32_t vertexStart, int32_t instanceBase) = 0;

		virtual void DoDrawIndexedInstanced(int32_t numIndicesPerInstance, int32_t numInstances, int32_t indexStart, int32_t indexBase, int32_t instanceBase) = 0;

		virtual void DoDrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset) = 0;

		virtual void DoCompute(int32_t groupX, int32_t groupY, int32_t groupZ) = 0;

		virtual void DoClearRenderTarget(const Ptr<RenderTargetView> & renderTarget, const float4 & color) = 0;

		virtual void DoClearDepthStencil(const Ptr<DepthStencilView> & depthStencil, float depth, uint8_t stencil) = 0;

	private:
		void UnBoundResource();

		void UpdateContext();
	};
}

#endif