#pragma once
#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\RenderViewport.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Shader;
	class BlendState;
	class DepthStencilState;
	class RasterizerState;
	class RenderBuffer;
	class ShaderProgram;
	class Sampler;
	class RenderInput;

	struct RenderContextState
	{
		Ptr<RenderInput> renderInput;
		RenderViewport viewport;
		std::vector<ResourceView> renderTargets;
		ResourceView depthStencil;
		std::array<Ptr<Shader>, ShaderTypeNum::NUM_SHADER_TYPE> shaders;

		Ptr<BlendState> blendState;
		std::vector<float> blendFactor;
		uint32_t blendSampleMask = 0xffffffff;
		Ptr<DepthStencilState> depthStencilState;
		uint32_t stencilRef = 0;
		Ptr<RasterizerState> rasterizerState;
	};

	enum RenderContextStateFlag : uint32_t
	{
		RENDER_CONTEXT_STATE_INPUT				= 1UL << 0UL,
		RENDER_CONTEXT_STATE_VIEWPORT			= 1UL << 1UL,
		RENDER_CONTEXT_STATE_RENDERTARGETS		= 1UL << 2UL,
		RENDER_CONTEXT_STATE_DEPTHSTENCIL		= 1UL << 3UL,
		RENDER_CONTEXT_STATE_SHADERS			= 1UL << 4UL,
		RENDER_CONTEXT_STATE_BLENDSTATE			= 1UL << 5UL,
		RENDER_CONTEXT_STATE_DEPTHSTENCILSTATE	= 1UL << 6UL,
		RENDER_CONTEXT_STATE_RASTERIZERSTATE	= 1UL << 7UL
	};

	struct RenderContextStateSave
	{
		uint32_t saveFlag;
		RenderContextState saveState;
	};


	class TOYGE_CORE_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		void SaveState(uint32_t saveFlag, RenderContextStateSave & outSave);

		void RestoreState(const RenderContextStateSave & stateSave);


		void SetViewport(const RenderViewport & viewport);

		const RenderViewport & GetViewport()
		{
			return _state.viewport;
		}


		void ClearRenderTargets(const std::vector<ResourceView> & renderTargets, const float4 & color)
		{
			DoClearRenderTargets(renderTargets, color);
		}

		void ClearRenderTargets(const float4 & color)
		{
			ClearRenderTargets(_state.renderTargets, color);
		}

		void ClearDepthStencil(const ResourceView & depthStencil, float depth, uint8_t stencil)
		{
			DoClearDepthStencil(depthStencil, depth, stencil);
		}

		void ClearDepthStencil(float depth, uint8_t stencil)
		{
			ClearDepthStencil(_state.depthStencil, depth, stencil);
		}


		void SetRenderTargets(const std::vector<ResourceView> & targets, int32_t offset);

		const std::vector<ResourceView> & GetRenderTargets() const
		{
			return _state.renderTargets;
		}


		void SetDepthStencil(const ResourceView & depthStencil);

		const ResourceView & GetDepthStencil() const
		{
			return _state.depthStencil;
		}


		void SetRenderInput(const Ptr<RenderInput> & input);

		Ptr<RenderInput> GetRenderInput() const
		{
			return _state.renderInput;
		}


		void SetShader(const Ptr<Shader> & shader);

		void ResetShader(ShaderType shaderType);

		const Ptr<Shader> GetShader(ShaderType shaderType) const;


		void SetBlendState(
			const Ptr<BlendState> & state,
			const std::vector<float> & blendFactors = {1.0f, 1.0f, 1.0f, 1.0f},
			uint32_t sampleMask = 0xffffffff);

		void SetDepthStencilState(const Ptr<DepthStencilState> & state, uint32_t steincilRef = 0);

		void SetRasterizerState(const Ptr<RasterizerState> & state);


		void DrawVertices(int32_t numVertices, int32_t vertexStart);

		void DrawVertices();

		void DrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase);

		void DrawIndexed();

		void DrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset);

		void Compute(int32_t groupX, int32_t groupY, int32_t groupZ);
		
	protected:
		RenderContextState _state;

		Ptr<RenderInput>				_inputCache;
		RenderViewport				_viewportCache;
		std::vector<ResourceView>	_rtsCache;
		ResourceView				_depthStencilCache;
		std::array<Ptr<ShaderProgram>, ShaderTypeNum::NUM_SHADER_TYPE>				_shaderProgramsCache;
		std::array<std::vector<ResourceView>, ShaderTypeNum::NUM_SHADER_TYPE>		_srsCache;
		std::array<std::vector<ResourceView>, ShaderTypeNum::NUM_SHADER_TYPE>		_uavsCache;
		std::array<std::vector<Ptr<RenderBuffer>>, ShaderTypeNum::NUM_SHADER_TYPE>	_bufsCache;
		std::array<std::vector<Ptr<Sampler>>, ShaderTypeNum::NUM_SHADER_TYPE>			_samsCache;
		Ptr<BlendState>			_bsCache;
		std::vector<float>		_bsFactorsCache;
		uint32_t				_bsSampleMaskCache;
		Ptr<DepthStencilState>	_dssCache;
		uint32_t				_stencilRefCache;
		Ptr<RasterizerState>		_rsCache;


		void UpdateContext();

		void UnBindResources();

		virtual void DoClearRenderTargets(const std::vector<ResourceView> & renderTargets, const float4 & color) = 0;

		virtual void DoClearDepthStencil(const ResourceView & depthStencil, float depth, uint8_t stencil) = 0;

		virtual void DoSetViewport(const RenderViewport & vp) = 0;

		virtual void DoSetRenderTargetsAndDepthStencil(const std::vector<ResourceView> & targets, const ResourceView & depthStencil) = 0;

		virtual void DoSetRenderInput(const Ptr<RenderInput> & input) = 0;

		virtual void DoSetShaderProgram(const Ptr<ShaderProgram> & program) = 0;

		virtual void DoResetShaderProgram(ShaderType shaderType) = 0;

		virtual void DoSetShaderResources(ShaderType shaderType, const std::vector<ResourceView> & resources, int32_t offset) = 0;

		virtual void DoSetRTsAndUAVs(const std::vector<ResourceView> & targets, const ResourceView & depthStencil, const std::vector<ResourceView> & uavs) = 0;

		virtual void DoSetUAVs(ShaderType shaderType, const std::vector<ResourceView> & resources, int32_t offset) = 0;

		virtual void DoSetShaderBuffers(ShaderType shaderType, const std::vector<Ptr<RenderBuffer>> & buffers, int32_t offset) = 0;

		virtual void DoSetShaderSamplers(ShaderType shaderType, const std::vector<Ptr<Sampler>> & samplers, int32_t offset) = 0;

		virtual void DoSetBlendState(const Ptr<BlendState> & state, const std::vector<float> & blendFactors, uint32_t sampleMask) = 0;

		virtual void DoSetDepthStencilState(const Ptr<DepthStencilState> & state, uint32_t steincilRef = 0) = 0;

		virtual void DoSetRasterizerState(const Ptr<RasterizerState> & state) = 0;

		virtual void DoDrawVertices(int32_t numVertices, int32_t vertexStart) = 0;

		virtual void DoDrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase) = 0;

		virtual void DoDrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset) = 0;

		virtual void DoCompute(int32_t groupX, int32_t groupY, int32_t groupZ) = 0;
	};
}

#endif