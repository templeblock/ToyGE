#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"
#include "ToyGE\RenderEngine\Sampler.h"
#include "ToyGE\RenderEngine\BlendState.h"
#include "ToyGE\RenderEngine\DepthStencilState.h"
#include "ToyGE\RenderEngine\RasterizerState.h"

namespace ToyGE
{
	void RenderContext::SetRenderTargets(const std::vector<Ptr<RenderTargetView>> & renderTargets)
	{
		_state.renderTargets = renderTargets;
		/*if(_state.renderTargets.size() < )
		_state.renderTargets.resize(offset + renderTargets.size());
		std::copy(renderTargets.begin(), renderTargets.end(), _state.renderTargets.begin() + offset);*/
	}

	void RenderContext::SetDepthStencil(const Ptr<DepthStencilView> & depthStencil)
	{
		_state.depthStencil = depthStencil;
	}

	void RenderContext::SetShader(const Ptr<ShaderProgram> & shader)
	{
		if (_state.shaders[shader->GetType()] != shader)
			_state.shaders[shader->GetType()] = shader;
	}

	void RenderContext::ResetShader(ShaderType shaderType)
	{
		if (_state.shaders[shaderType])
			_state.shaders[shaderType] = nullptr;
		_state.cbs[shaderType].clear();
		_state.srvs[shaderType].clear();
		_state.samplers[shaderType].clear();
		_state.uavs[shaderType].clear();
		_state.uavInitialCounts[shaderType].clear();
	}

	Ptr<ShaderProgram> RenderContext::GetShader(ShaderType shaderType) const
	{
		return _state.shaders[shaderType];
	}

	void RenderContext::SetSRVs(ShaderType shaderType, const std::vector<Ptr<ShaderResourceView>> & srvs)
	{
		_state.srvs[shaderType] = srvs;
		/*if(_state.srvs[shaderType].size() < static_cast<size_t>(offset) + srvs.size())
			_state.srvs[shaderType].resize(offset + srvs.size());
		std::copy(srvs.begin(), srvs.end(), _state.srvs[shaderType].begin() + offset);*/
	}

	void RenderContext::SetUAVs(ShaderType shaderType, const std::vector<Ptr<UnorderedAccessView>> & uavs, const std::vector<int32_t> & uavInitialCounts)
	{
		/*if (_state.uavs[shaderType].size() < static_cast<size_t>(offset) + uavs.size())
			_state.uavs[shaderType].resize(offset + uavs.size());
		std::copy(uavs.begin(), uavs.end(), _state.uavs[shaderType].begin() + offset);

		if (_state.uavInitialCounts[shaderType].size() < static_cast<size_t>(offset) + uavInitialCounts.size())
			_state.uavInitialCounts[shaderType].resize(offset + uavInitialCounts.size());
		std::copy(uavInitialCounts.begin(), uavInitialCounts.end(), _state.uavInitialCounts[shaderType].begin() + offset);*/

		_state.uavs[shaderType] = uavs;
		_state.uavInitialCounts[shaderType] = uavInitialCounts;

		_state.uavInitialCounts[shaderType].resize(_state.uavs[shaderType].size());
	}

	void RenderContext::SetSamplers(ShaderType shaderType, const std::vector<Ptr<Sampler>> & samplers)
	{
		/*if (_state.samplers[shaderType].size() < static_cast<size_t>(offset) + samplers.size())
			_state.samplers[shaderType].resize(offset + samplers.size());
		std::copy(samplers.begin(), samplers.end(), _state.samplers[shaderType].begin() + offset);*/
		_state.samplers[shaderType] = samplers;
	}

	void RenderContext::SetCBs(ShaderType shaderType, const std::vector<Ptr<RenderBuffer>> & cbs)
	{
		/*if (_state.cbs[shaderType].size() < static_cast<size_t>(offset) + cbs.size())
			_state.cbs[shaderType].resize(offset + cbs.size());
		std::copy(cbs.begin(), cbs.end(), _state.cbs[shaderType].begin() + offset);*/
		_state.cbs[shaderType] = cbs;
	}

	void RenderContext::SetBlendState(const Ptr<BlendState> & state, const float4 & blendFactors, uint32_t sampleMask)
	{
		_state.blendState = state;
		_state.blendFactor = blendFactors;
		_state.blendSampleMask = sampleMask;
	}

	void RenderContext::SetDepthStencilState(const Ptr<DepthStencilState> & state, uint32_t steincilRef)
	{
		_state.depthStencilState = state;
		_state.stencilRef = steincilRef;
	}

	void RenderContext::SetRasterizerState(const Ptr<RasterizerState> & state)
	{
		_state.rasterizerState = state;
	}

	void RenderContext::DrawVertices(int32_t numVertices, int32_t vertexStart)
	{
		UpdateContext();
		DoDrawVertices(numVertices, vertexStart);
	}

	void RenderContext::DrawVertices(int32_t vertexStart)
	{
		if(_state.vertexBuffers.size() > 0)
			DrawVertices(_state.vertexBuffers[0]->GetDesc().numElements, vertexStart);
	}

	void RenderContext::DrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase)
	{
		UpdateContext();
		DoDrawIndexed(numIndices, indexStart, indexBase);
	}

	void RenderContext::DrawIndexed(int32_t indexStart, int32_t indexBase)
	{
		if(_state.indexBuffer)
			DrawIndexed(_state.indexBuffer->GetDesc().numElements, indexStart, indexBase);
	}

	void RenderContext::DrawIndexedInstanced(int32_t numIndicesPerInstance, int32_t numInstances, int32_t indexStart, int32_t indexBase, int32_t instanceBase)
	{
		UpdateContext();
		DoDrawIndexedInstanced(numIndicesPerInstance, numInstances, indexStart, indexBase, instanceBase);
	}

	void RenderContext::DrawInstanced(int32_t numVerticesPerInstance, int32_t numInstances, int32_t vertexStart, int32_t instanceBase)
	{
		UpdateContext();
		DoDrawInstanced(numVerticesPerInstance, numInstances, vertexStart, instanceBase);
	}

	void RenderContext::DrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset)
	{
		UpdateContext();
		DoDrawInstancedIndirect(indirectArgsBuffer, bytesOffset);
	}

	void RenderContext::Compute(int32_t gropuX, int32_t groupY, int32_t groupZ)
	{
		ResetShader(SHADER_VS);
		ResetShader(SHADER_PS);
		ResetShader(SHADER_GS);
		ResetShader(SHADER_DS);
		ResetShader(SHADER_HS);

		SetRenderTargets({});
		SetDepthStencil(nullptr);

		SetVertexBuffer({});
		SetIndexBuffer(nullptr);

		SetBlendState(nullptr);
		SetDepthStencilState(nullptr);
		SetRasterizerState(nullptr);

		UpdateContext();
		DoCompute(gropuX, groupY, groupZ);
	}

	void RenderContext::UnBoundResource()
	{
		// Unbound outputs(render target/depth stencil/uav) if are currently bound as inputs

		// RTVs
		for (auto & rtv : _state.renderTargets)
		{
			auto rtvRes = rtv->GetResource();

			if (rtvRes && rtvRes->IsBound() && rtvRes->IsBoundAsInput())
			{
				if (rtvRes->GetBoundState().boundType == RenderResourceBoundType::RRBT_SRV)
				{
					DoSetSRVs(rtvRes->GetBoundState().boundShaderType, { nullptr }, rtvRes->GetBoundState().boundIndex);
					_stateCache.srvs[rtvRes->GetBoundState().boundShaderType][rtvRes->GetBoundState().boundIndex] = nullptr;

					rtvRes->ResetBound();
				}
			}

			if (rtvRes && rtvRes->IsBound())
			{
				if (rtvRes->GetBoundState().boundType == RenderResourceBoundType::RRBT_UAV)
				{
					DoSetUAVs(rtvRes->GetBoundState().boundShaderType, { nullptr }, {0}, rtvRes->GetBoundState().boundIndex);
					_stateCache.uavs[rtvRes->GetBoundState().boundShaderType][rtvRes->GetBoundState().boundIndex] = nullptr;

					rtvRes->ResetBound();
				}
			}
		}

		// DSV
		if (_state.depthStencil)
		{
			auto dsvRes = _state.depthStencil->GetResource();
			if (dsvRes && dsvRes->IsBound() && dsvRes->IsBoundAsInput())
			{
				if (dsvRes->GetBoundState().boundType == RenderResourceBoundType::RRBT_SRV)
				{
					DoSetSRVs(dsvRes->GetBoundState().boundShaderType, { nullptr }, dsvRes->GetBoundState().boundIndex);
					_stateCache.srvs[dsvRes->GetBoundState().boundShaderType][dsvRes->GetBoundState().boundIndex] = nullptr;

					dsvRes->ResetBound();
				}
			}

			if (dsvRes && dsvRes->IsBound())
			{
				if (dsvRes->GetBoundState().boundType == RenderResourceBoundType::RRBT_UAV)
				{
					DoSetUAVs(dsvRes->GetBoundState().boundShaderType, { nullptr }, { 0 }, dsvRes->GetBoundState().boundIndex);
					_stateCache.uavs[dsvRes->GetBoundState().boundShaderType][dsvRes->GetBoundState().boundIndex] = nullptr;

					dsvRes->ResetBound();
				}
			}
		}

		// UAVs
		for (auto & uavList : _state.uavs)
		{
			for (auto & uav : uavList)
			{
				if (uav)
				{
					auto uavRes = uav->GetResource();
					if (uavRes && uavRes->IsBound() && uavRes->IsBoundAsInput())
					{
						if (uavRes->GetBoundState().boundType == RenderResourceBoundType::RRBT_SRV)
						{
							DoSetSRVs(uavRes->GetBoundState().boundShaderType, { nullptr }, uavRes->GetBoundState().boundIndex);
							_stateCache.srvs[uavRes->GetBoundState().boundShaderType][uavRes->GetBoundState().boundIndex] = nullptr;

							uavRes->ResetBound();
						}
					}
				}
			}
		}
	}

	void RenderContext::UpdateContext()
	{
		UnBoundResource();

		// Viewport
		if (_state.viewport != _stateCache.viewport)
		{
			DoSetViewport(_state.viewport);
		}

		// Vertex buffer
		if (_state.vertexBuffers != _stateCache.vertexBuffers || _state.vertexBufferOffsets != _stateCache.vertexBufferOffsets)
		{
			for (auto & vb : _stateCache.vertexBuffers)
			{
				if (vb)
					vb->ResetBound();
			}

			DoSetVertexBuffer(_state.vertexBuffers, _state.vertexBufferOffsets);

			int32_t index = 0;
			for (auto & vb : _state.vertexBuffers)
			{
				if (vb)
					vb->SetBound(RenderResourceBoundType::RRBT_VB, SHADER_VS, index++);
			}
		}

		// Index buffer
		if (_state.indexBuffer != _stateCache.indexBuffer || _state.indexBufferOffset != _stateCache.indexBufferOffset)
		{
			if (_stateCache.indexBuffer)
				_stateCache.indexBuffer->ResetBound();

			DoSetIndexBuffer(_state.indexBuffer, _state.indexBufferOffset);

			if(_state.indexBuffer)
				_state.indexBuffer->SetBound(RenderResourceBoundType::RRBT_IB, SHADER_VS, 0);
		}

		// PT
		if (_state.primitiveTopology != _stateCache.primitiveTopology)
		{
			DoSetPrimitiveTopology(_state.primitiveTopology);
		}

		// Vertex input layout
		if (_state.shaders[SHADER_VS] != _stateCache.shaders[SHADER_VS] || _state.vertexBuffers != _stateCache.vertexBuffers)
		{
			if(_state.shaders[SHADER_VS] && _state.vertexBuffers.size() > 0)
				DoSetVertexInputLayout(VertexInputLayout::GetVertexInputLayout(_state.shaders[SHADER_VS], _state.vertexBuffers));
			else
				DoSetVertexInputLayout(nullptr);
		}

		// RTVs and DSV
		bool bNeedUpdatePsUavs = _state.uavs[SHADER_PS] != _stateCache.uavs[SHADER_PS] ||
			_state.uavInitialCounts[SHADER_PS].size() > 0;

		if (_state.renderTargets != _stateCache.renderTargets || 
			_state.depthStencil != _stateCache.depthStencil || 
			bNeedUpdatePsUavs)
		{
			auto rtvs = _state.renderTargets;
			while (rtvs.size() < _stateCache.renderTargets.size())
				rtvs.push_back(nullptr);

			for (auto & rtvReset : _stateCache.renderTargets)
			{
				if (rtvReset && rtvReset->GetResource())
					rtvReset->GetResource()->ResetBound();
			}

			if (_stateCache.depthStencil && _stateCache.depthStencil->GetResource())
				_stateCache.depthStencil->GetResource()->ResetBound();

			// uavs
			if (bNeedUpdatePsUavs)
			{
				auto uavs = _state.uavs[SHADER_PS];
				auto initalCounts = _state.uavInitialCounts[SHADER_PS];
				while (uavs.size() < _stateCache.uavs[SHADER_PS].size())
				{
					uavs.push_back(nullptr);
					initalCounts.push_back(0);
				}

				for (auto & uavReset : _stateCache.uavs[SHADER_PS])
				{
					if (uavReset && uavReset->GetResource())
						uavReset->GetResource()->ResetBound();
				}

				DoSetRTVsAndUAVs(rtvs, _state.depthStencil, uavs, initalCounts);
			}
			else
			{
				DoSetRenderTargetsAndDepthStencil(rtvs, _state.depthStencil);
			}

			// Set Bound
			for (int32_t i = 0; i != static_cast<int32_t>(_state.renderTargets.size()); ++i)
			{
				if (_state.renderTargets[i] && _state.renderTargets[i]->GetResource())
					_state.renderTargets[i]->GetResource()->SetBound(RenderResourceBoundType::RRBT_RTV, SHADER_PS, i);
			}

			if (_state.depthStencil && _state.depthStencil->GetResource())
				_state.depthStencil->GetResource()->SetBound(RenderResourceBoundType::RRBT_DSV, SHADER_PS, 0);

			for (int32_t i = 0; i != static_cast<int32_t>(_state.uavs[SHADER_PS].size()); ++i)
			{
				if (_state.uavs[SHADER_PS][i] && _state.uavs[SHADER_PS][i]->GetResource())
					_state.uavs[SHADER_PS][i]->GetResource()->SetBound(RenderResourceBoundType::RRBT_UAV, SHADER_PS, i);
			}
		}

		// Shaders
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM; ++shaderType)
		{
			if (_state.shaders[shaderType] != _stateCache.shaders[shaderType])
			{
				DoSetShaderProgram(static_cast<ShaderType>(shaderType), _state.shaders[shaderType]);
			}
		}

		// UAVs
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM; ++shaderType)
		{
			if (static_cast<ShaderType>(shaderType) == SHADER_PS)
				continue;

			if (_state.uavs[shaderType] != _stateCache.uavs[shaderType] || _state.uavInitialCounts[shaderType].size() > 0)
			{
				auto uavs = _state.uavs[shaderType];
				auto initalCounts = _state.uavInitialCounts[shaderType];
				while (uavs.size() < _stateCache.uavs[shaderType].size())
				{
					uavs.push_back(nullptr);
					initalCounts.push_back(0);
				}

				for (auto & uavReset : _stateCache.uavs[shaderType])
				{
					if (uavReset && uavReset->GetResource())
						uavReset->GetResource()->ResetBound();
				}

				DoSetUAVs(static_cast<ShaderType>(shaderType), uavs, initalCounts, 0);

				for (int32_t i = 0; i != static_cast<int32_t>(_state.uavs[shaderType].size()); ++i)
				{
					if (_state.uavs[shaderType][i] && _state.uavs[shaderType][i]->GetResource())
						_state.uavs[shaderType][i]->GetResource()->SetBound(RenderResourceBoundType::RRBT_UAV, static_cast<ShaderType>(shaderType), i);
				}
			}
		}

		// SRVs
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM; ++shaderType)
		{
			if (_state.srvs[shaderType] != _stateCache.srvs[shaderType])
			{
				auto srvs = _state.srvs[shaderType];
				while (srvs.size() < _stateCache.srvs[shaderType].size())
					srvs.push_back(nullptr);

				for (auto & srvReset : _stateCache.srvs[shaderType])
				{
					if (srvReset && srvReset->GetResource())
						srvReset->GetResource()->ResetBound();
				}

				DoSetSRVs(static_cast<ShaderType>(shaderType), srvs, 0);

				for (int32_t i = 0; i != static_cast<int32_t>(_state.srvs[shaderType].size()); ++i)
				{
					if (_state.srvs[shaderType][i] && _state.srvs[shaderType][i]->GetResource())
						_state.srvs[shaderType][i]->GetResource()->SetBound(RenderResourceBoundType::RRBT_SRV, static_cast<ShaderType>(shaderType), i);
				}
			}
		}

		// CBs
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM; ++shaderType)
		{
			if (_state.cbs[shaderType] != _stateCache.cbs[shaderType])
			{
				auto cbs = _state.cbs[shaderType];
				while (cbs.size() < _stateCache.cbs[shaderType].size())
					cbs.push_back(nullptr);

				for (auto & cbReset : _stateCache.cbs[shaderType])
				{
					if (cbReset)
						cbReset->ResetBound();
				}

				DoSetCBs(static_cast<ShaderType>(shaderType), cbs, 0);

				for (int32_t i = 0; i != static_cast<int32_t>(_state.cbs[shaderType].size()); ++i)
				{
					if (_state.cbs[shaderType][i])
						_state.cbs[shaderType][i]->SetBound(RenderResourceBoundType::RRBT_CB, static_cast<ShaderType>(shaderType), i);
				}
			}
		}

		// Samplers
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM; ++shaderType)
		{
			if (_state.samplers[shaderType] != _stateCache.samplers[shaderType])
			{
				auto samplers = _state.samplers[shaderType];
				while (samplers.size() < _stateCache.samplers[shaderType].size())
					samplers.push_back(nullptr);

				for (auto & samplerReset : _stateCache.samplers[shaderType])
				{
					if (samplerReset)
						samplerReset->ResetBound();
				}

				DoSetSamplers(static_cast<ShaderType>(shaderType), samplers, 0);

				for (int32_t i = 0; i != static_cast<int32_t>(_state.samplers[shaderType].size()); ++i)
				{
					if (_state.samplers[shaderType][i])
						_state.samplers[shaderType][i]->SetBound(RenderResourceBoundType::RRBT_SAMPLER, static_cast<ShaderType>(shaderType), i);
				}
			}
		}

		// BlendState
		if (_state.blendState != _stateCache.blendState || 
			any(_state.blendFactor != _stateCache.blendFactor) || 
			_state.blendSampleMask != _stateCache.blendSampleMask)
		{
			if (_stateCache.blendState)
				_stateCache.blendState->ResetBound();

			DoSetBlendState(_state.blendState, _state.blendFactor, _state.blendSampleMask);

			if (_state.blendState)
				_state.blendState->SetBound(RenderResourceBoundType::RRBT_STATE, SHADER_PS, 0);
		}

		// DepthStencilState
		if (_state.depthStencilState != _stateCache.depthStencilState || _state.stencilRef != _stateCache.stencilRef)
		{
			if (_stateCache.depthStencilState)
				_stateCache.depthStencilState->ResetBound();

			DoSetDepthStencilState(_state.depthStencilState, _state.stencilRef);

			if (_state.depthStencilState)
				_state.depthStencilState->SetBound(RenderResourceBoundType::RRBT_STATE, SHADER_PS, 0);
		}

		// RasterizerState
		if (_state.rasterizerState != _stateCache.rasterizerState)
		{
			if (_stateCache.rasterizerState)
				_stateCache.rasterizerState->ResetBound();

			DoSetRasterizerState(_state.rasterizerState);

			if (_state.rasterizerState)
				_state.rasterizerState->SetBound(RenderResourceBoundType::RRBT_STATE, SHADER_PS, 0);
		}

		// Update cache
		_stateCache = _state;
		for (auto & initalCounts : _state.uavInitialCounts)
			initalCounts.clear();
	}
}