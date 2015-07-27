#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Kernel\Timer.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"

namespace ToyGE
{
	void RenderContext::SaveState(uint32_t saveFlag, RenderContextStateSave & outSave)
	{
		outSave.saveFlag = saveFlag;
		if (saveFlag & RENDER_CONTEXT_STATE_INPUT)
			outSave.saveState.renderInput = _state.renderInput;
		if (saveFlag & RENDER_CONTEXT_STATE_VIEWPORT)
			outSave.saveState.viewport = _state.viewport;
		if (saveFlag & RENDER_CONTEXT_STATE_RENDERTARGETS)
			outSave.saveState.renderTargets = _state.renderTargets;
		if (saveFlag & RENDER_CONTEXT_STATE_DEPTHSTENCIL)
			outSave.saveState.depthStencil = _state.depthStencil;
		if (saveFlag & RENDER_CONTEXT_STATE_SHADERS)
			outSave.saveState.shaders = _state.shaders;
		if (saveFlag & RENDER_CONTEXT_STATE_BLENDSTATE)
		{
			outSave.saveState.blendState = _state.blendState;
			outSave.saveState.blendFactor = _state.blendFactor;
			outSave.saveState.blendSampleMask = _state.blendSampleMask;
		}
		if (saveFlag & RENDER_CONTEXT_STATE_DEPTHSTENCILSTATE)
		{
			outSave.saveState.depthStencilState = _state.depthStencilState;
			outSave.saveState.stencilRef = _state.stencilRef;
		}
		if (saveFlag & RENDER_CONTEXT_STATE_RASTERIZERSTATE)
			outSave.saveState.rasterizerState = _state.rasterizerState;
	}

	void RenderContext::RestoreState(const RenderContextStateSave & stateSave)
	{
		auto saveFlag = stateSave.saveFlag;
		if (saveFlag & RENDER_CONTEXT_STATE_INPUT)
			_state.renderInput = stateSave.saveState.renderInput;
		if (saveFlag & RENDER_CONTEXT_STATE_VIEWPORT)
			_state.viewport = stateSave.saveState.viewport;
		if (saveFlag & RENDER_CONTEXT_STATE_RENDERTARGETS)
			_state.renderTargets = stateSave.saveState.renderTargets;
		if (saveFlag & RENDER_CONTEXT_STATE_DEPTHSTENCIL)
			_state.depthStencil = stateSave.saveState.depthStencil;
		if (saveFlag & RENDER_CONTEXT_STATE_SHADERS)
			_state.shaders = stateSave.saveState.shaders;
		if (saveFlag & RENDER_CONTEXT_STATE_BLENDSTATE)
		{
			_state.blendState = stateSave.saveState.blendState;
			_state.blendFactor = stateSave.saveState.blendFactor;
			_state.blendSampleMask = stateSave.saveState.blendSampleMask;
		}
		if (saveFlag & RENDER_CONTEXT_STATE_DEPTHSTENCILSTATE)
		{
			_state.depthStencilState = stateSave.saveState.depthStencilState;
			_state.stencilRef = stateSave.saveState.stencilRef;
		}
		if (saveFlag & RENDER_CONTEXT_STATE_RASTERIZERSTATE)
			_state.rasterizerState = stateSave.saveState.rasterizerState;
	}

	void RenderContext::SetViewport(const RenderViewport & viewport)
	{
		_state.viewport = viewport;
	}

	void RenderContext::SetRenderTargets(const std::vector<ResourceView> & targets, int32_t offset)
	{
		_state.renderTargets.resize(offset + targets.size());
		std::copy(targets.begin(), targets.end(), _state.renderTargets.begin() + offset);
	}

	void RenderContext::SetDepthStencil(const ResourceView & depthStencil)
	{
		_state.depthStencil = depthStencil;
	}

	void RenderContext::SetRenderInput(const Ptr<RenderInput> & input)
	{
		if (_state.renderInput == input)
			return;

		_state.renderInput = input;
	}

	void RenderContext::SetShader(const Ptr<Shader> & shader)
	{
		if (_state.shaders[shader->GetProgram()->Type()] != shader)
		{
			_state.shaders[shader->GetProgram()->Type()] = shader;
		}
	}

	void RenderContext::ResetShader(ShaderType shaderType)
	{
		if (_state.shaders[shaderType])
		{
			_state.shaders[shaderType].reset();
		}
	}

	const Ptr<Shader> RenderContext::GetShader(ShaderType shaderType) const
	{
		return _state.shaders[shaderType];
	}

	void RenderContext::SetBlendState(const Ptr<BlendState> & state, const std::vector<float> & blendFactors, uint32_t sampleMask)
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

	void RenderContext::DrawVertices()
	{
		if (_state.renderInput)
			DrawVertices(_state.renderInput->NumVertices(), _state.renderInput->GetVertexStart());
	}

	void RenderContext::DrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase)
	{
		UpdateContext();
		DoDrawIndexed(numIndices, indexStart, indexBase);
		//auto & debugInfo = DebugInfo::get_mutable_instance();
		//DebugInfo::Instance()->AddValue<int>("DrawPrimitives", numIndices);
		//DebugInfo::Instance()->AddValue<int>("DrawCalls", 1);
	}

	void RenderContext::DrawIndexed()
	{
		DrawIndexed(_state.renderInput->NumIndices(), _state.renderInput->GetIndexStart(), _state.renderInput->GetVertexStart());
	}

	void RenderContext::DrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset)
	{
		UpdateContext();
		DoDrawInstancedIndirect(indirectArgsBuffer, bytesOffset);
	}

	void RenderContext::Compute(int32_t gropuX, int32_t groupY, int32_t groupZ)
	{
		UpdateContext();
		DoCompute(gropuX, groupY, groupZ);
	}

	void RenderContext::UpdateContext()
	{
		//unbind render targets which are bound to other stages
		UnBindResources();

		//update viewport
		if (_viewportCache != _state.viewport)
		{
			DoSetViewport(_state.viewport);
			_viewportCache = _state.viewport;
		}

		//update render targets and depth stencil
		bool bPSUAVChanged = false;
		bPSUAVChanged =
			(_state.shaders[SHADER_PS] && _state.shaders[SHADER_PS]->_uavs != _uavsCache[SHADER_PS])
			|| (_state.shaders[SHADER_PS] == nullptr && _uavsCache[SHADER_PS].size() > 0);

		if (_state.renderTargets != _rtsCache || _state.depthStencil != _depthStencilCache || bPSUAVChanged)
		{
			std::vector<ResourceView> rts;
			rts = _state.renderTargets;
			if (_rtsCache.size() > rts.size())
			{
				int32_t nullCnt = static_cast<int32_t>(_rtsCache.size() - rts.size());
				for (int32_t i = 0; i < nullCnt; ++i)
				{
					rts.push_back(ResourceView());
				}
			}
			for (auto & resetRts : _rtsCache)
			{
				if (resetRts.resource)
					resetRts.resource->ResetBind();
			}
			if (_depthStencilCache.resource)
				_depthStencilCache.resource->ResetBind();

			//UAVs For PS
			std::vector<ResourceView> uavs;
			if (bPSUAVChanged)
			{
				if (_state.shaders[SHADER_PS])
				{
					uavs = _state.shaders[SHADER_PS]->_uavs;

					if (_uavsCache[SHADER_PS].size() > uavs.size())
					{
						int32_t nullCnt = static_cast<int32_t>(_uavsCache[SHADER_PS].size() - uavs.size());
						int32_t uavOffset = static_cast<int32_t>(uavs.size());
						for (int32_t j = 0; j != nullCnt; ++j)
						{
							uavs.push_back(ResourceView());
						}
					}
					for (auto & resetUAV : _uavsCache[SHADER_PS])
					{
						if (resetUAV.resource)
							resetUAV.resource->ResetBind();
					}
				}
				else
				{
					uavs = std::vector<ResourceView>(_uavsCache[SHADER_PS].size(), ResourceView());
					for (auto & resetView : _uavsCache[SHADER_PS])
					{
						if (resetView.resource)
							resetView.resource->ResetBind();
					}
				}
			}

			//Do Update 
			if (uavs.size() <= 0)
				DoSetRenderTargetsAndDepthStencil(rts, _state.depthStencil);
			else
				DoSetRTsAndUAVs(rts, _state.depthStencil, uavs);

			//Set Bind
			for (int32_t i = 0; i != static_cast<int32_t>(_state.renderTargets.size()); ++i)
			{
				if (_state.renderTargets[i].resource)
					_state.renderTargets[i].resource->SetBind(RESOURCE_BOUND_RENDER_TARGET, i);
			}

			if (_state.depthStencil.resource)
				_state.depthStencil.resource->SetBind(RESOURCE_BOUND_DEPTH_STENCIL);

			int32_t uavBoundIndex = 0;
			for (auto & uav : uavs)
			{
				if (uav.resource)
					uav.resource->SetBind(RESOURCE_BOUND_UAV, uavBoundIndex++, SHADER_PS, uav.subDesc);
			}

			//Update Cache
			_rtsCache = _state.renderTargets;
			_depthStencilCache = _state.depthStencil;
			_uavsCache[SHADER_PS] = uavs;
		}

		//update input
		if (_state.renderInput != _inputCache && _state.shaders[SHADER_VS])
		{
			DoSetRenderInput(_state.renderInput);
			_inputCache = _state.renderInput;
		}

		//update shader progrmas
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM_SHADER_TYPE; ++shaderType)
		{
			if (_state.shaders[shaderType])
			{
				if (_shaderProgramsCache[shaderType] != _state.shaders[shaderType]->GetProgram())
				{
					DoSetShaderProgram(_state.shaders[shaderType]->GetProgram());
					_shaderProgramsCache[shaderType] = _state.shaders[shaderType]->GetProgram();
				}
			}
			else
			{
				if (_shaderProgramsCache[shaderType])
				{
					DoResetShaderProgram(static_cast<ShaderType>(shaderType));
					_shaderProgramsCache[shaderType].reset();
				}
			}
		}

		//update uavs
		//for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::value; ++shaderType)
		//{
		if (_state.shaders[SHADER_CS])
		{
			std::vector<ResourceView> uavs;
			uavs = _state.shaders[SHADER_CS]->_uavs;
			if (uavs != _uavsCache[SHADER_CS])
			{
				if (_uavsCache[SHADER_CS].size() > uavs.size())
				{
					int32_t nullCnt = static_cast<int32_t>(_uavsCache[SHADER_CS].size() - uavs.size());
					int32_t uavOffset = static_cast<int32_t>(uavs.size());
					for (int32_t j = 0; j != nullCnt; ++j)
					{
						uavs.push_back(ResourceView());
					}
				}
				for (auto & resetUAV : _uavsCache[SHADER_CS])
				{
					if (resetUAV.resource)
						resetUAV.resource->ResetBind();
				}

				DoSetUAVs(static_cast<ShaderType>(SHADER_CS), uavs, 0);
				for (int32_t uavIndex = 0; uavIndex != _state.shaders[SHADER_CS]->NumUAVs(); ++uavIndex)
				{
					auto setView = _state.shaders[SHADER_CS]->GetUAV(uavIndex);
					if (setView.resource)
						setView.resource->SetBind(RESOURCE_BOUND_UAV, uavIndex, static_cast<ShaderType>(SHADER_CS), setView.subDesc);
				}
				_uavsCache[SHADER_CS] = _state.shaders[SHADER_CS]->_uavs;
			}
		}
		else
		{
			if (_uavsCache[SHADER_CS].size() > 0)
			{
				std::vector<ResourceView> uavs(_uavsCache[SHADER_CS].size(), ResourceView());
				DoSetUAVs(static_cast<ShaderType>(SHADER_CS), uavs, 0);
				for (auto & resetView : _uavsCache[SHADER_CS])
				{
					if (resetView.resource)
						resetView.resource->ResetBind();
				}
				_uavsCache[SHADER_CS].clear();
			}
		}
		//}

		//update shader resources
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM_SHADER_TYPE; ++shaderType)
		{
			if (_state.shaders[shaderType])
			{
				std::vector<ResourceView> srs;
				srs = _state.shaders[shaderType]->_srs;
				if (srs != _srsCache[shaderType])
				{
					if (_srsCache[shaderType].size() > srs.size())
					{
						int32_t nullCnt = static_cast<int32_t>(_srsCache[shaderType].size() - srs.size());
						int32_t srsOffset = static_cast<int32_t>(srs.size());
						for (int32_t j = 0; j != nullCnt; ++j)
						{
							srs.push_back(ResourceView());
						}
					}
					for (auto & resetSR : _srsCache[shaderType])
					{
						if (resetSR.resource)
							resetSR.resource->ResetBind();
					}

					DoSetShaderResources(static_cast<ShaderType>(shaderType), srs, 0);
					for (int32_t srIndex = 0; srIndex != _state.shaders[shaderType]->NumShaderResources(); ++srIndex)
					{
						auto setView = _state.shaders[shaderType]->GetShaderResource(srIndex);
						if (setView.resource)
							setView.resource->SetBind(RESOURCE_BOUND_SHADER_RESOURCE, srIndex, static_cast<ShaderType>(shaderType), setView.subDesc);
					}
					_srsCache[shaderType] = _state.shaders[shaderType]->_srs;
				}
			}
			else
			{
				if (_srsCache[shaderType].size() > 0)
				{
					std::vector<ResourceView> srs(_srsCache[shaderType].size(), ResourceView());
					DoSetShaderResources(static_cast<ShaderType>(shaderType), srs, 0);
					for (auto & resetView : _srsCache[shaderType])
					{
						if (resetView.resource)
							resetView.resource->ResetBind();
					}
					_srsCache[shaderType].clear();
				}
			}
		}

		//update shader buffers
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM_SHADER_TYPE; ++shaderType)
		{
			if (_state.shaders[shaderType])
			{
				std::vector<Ptr<RenderBuffer>> bufs;
				bufs = _state.shaders[shaderType]->_cbs;
				if (bufs != _bufsCache[shaderType])
				{
					if (_bufsCache[shaderType].size() > bufs.size())
					{
						int32_t nullCnt = static_cast<int32_t>(_bufsCache[shaderType].size() - bufs.size());
						int32_t bufsOffset = static_cast<int32_t>(bufs.size());
						for (int32_t j = 0; j != nullCnt; ++j)
						{
							bufs.push_back(Ptr<RenderBuffer>());
						}
					}
					for (auto & resetBuf : _bufsCache[shaderType])
					{
						if (resetBuf)
							resetBuf->ResetBind();
					}

					DoSetShaderBuffers(static_cast<ShaderType>(shaderType), bufs, 0);
					for (int32_t bufIndex = 0; bufIndex != _state.shaders[shaderType]->NumConstantBuffers(); ++bufIndex)
					{
						auto setBuf = _state.shaders[shaderType]->GetConstantBuffer(bufIndex);
						if (setBuf)
							setBuf->SetBind(RESOURCE_BOUND_SHADER_BUFFER, bufIndex, static_cast<ShaderType>(shaderType));
					}
					_bufsCache[shaderType] = _state.shaders[shaderType]->_cbs;
				}
			}
			else
			{
				if (_bufsCache[shaderType].size() > 0)
				{
					std::vector<Ptr<RenderBuffer>> bufs(_bufsCache[shaderType].size(), Ptr<RenderBuffer>());
					DoSetShaderBuffers(static_cast<ShaderType>(shaderType), bufs, 0);
					for (auto & resetBuf : _bufsCache[shaderType])
					{
						if (resetBuf)
							resetBuf->ResetBind();
					}
					_bufsCache[shaderType].clear();
				}
			}
		}

		//update samplers
		for (uint32_t shaderType = 0; shaderType != ShaderTypeNum::NUM_SHADER_TYPE; ++shaderType)
		{
			if (_state.shaders[shaderType])
			{
				if (_state.shaders[shaderType]->_samplers != _samsCache[shaderType])
				{
					DoSetShaderSamplers(static_cast<ShaderType>(shaderType), _state.shaders[shaderType]->_samplers, 0);
					_samsCache[shaderType] = _state.shaders[shaderType]->_samplers;
				}
			}
		}

		//update blend state
		if (_state.blendState != _bsCache || _state.blendFactor != _bsFactorsCache || _state.blendSampleMask != _bsSampleMaskCache)
		{
			DoSetBlendState(_state.blendState, _state.blendFactor, _state.blendSampleMask);
			_bsCache = _state.blendState;
			_bsFactorsCache = _state.blendFactor;
			_bsSampleMaskCache = _state.blendSampleMask;
		}

		//update depth stencil state
		if (_state.depthStencilState != _dssCache || _state.stencilRef != _stencilRefCache)
		{
			DoSetDepthStencilState(_state.depthStencilState, _state.stencilRef);
			_dssCache = _state.depthStencilState;
			_stencilRefCache = _state.stencilRef;
		}

		//update rasterizer state
		if (_state.rasterizerState != _rsCache)
		{
			DoSetRasterizerState(_state.rasterizerState);
			_rsCache = _state.rasterizerState;
		}
	}

	void RenderContext::UnBindResources()
	{
		std::set<ShaderType> srUnbindSet;
		std::set<ShaderType> bufUnbindSet;
		for (auto & rt : _state.renderTargets)
		{
			if (rt.resource && rt.resource->IsBinded() &&
				(RESOURCE_BOUND_RENDER_TARGET != rt.resource->BoundState()))
			{
				if (RESOURCE_BOUND_SHADER_RESOURCE == rt.resource->BoundState())
					srUnbindSet.insert(rt.resource->BoundShaderType());
				else if (RESOURCE_BOUND_SHADER_BUFFER == rt.resource->BoundState())
					bufUnbindSet.insert(rt.resource->BoundShaderType());
			}
		}

		if (_state.depthStencil.resource)
		{
			if (_state.depthStencil.resource->IsBinded() &&
				(RESOURCE_BOUND_DEPTH_STENCIL != _state.depthStencil.resource->BoundState()))
			{
				if (RESOURCE_BOUND_SHADER_RESOURCE == _state.depthStencil.resource->BoundState())
					srUnbindSet.insert(_state.depthStencil.resource->BoundShaderType());
				else if (RESOURCE_BOUND_SHADER_BUFFER == _state.depthStencil.resource->BoundState())
					bufUnbindSet.insert(_state.depthStencil.resource->BoundShaderType());
			}
		}

		if (_state.shaders[SHADER_CS])
		{
			for (auto & uav : _state.shaders[SHADER_CS]->_uavs)
			{
				if (uav.resource && uav.resource->IsBinded() &&
					(RESOURCE_BOUND_UAV != uav.resource->BoundState()))
				{
					if (RESOURCE_BOUND_SHADER_RESOURCE == uav.resource->BoundState())
						srUnbindSet.insert(uav.resource->BoundShaderType());
					else if (RESOURCE_BOUND_SHADER_BUFFER == uav.resource->BoundState())
						bufUnbindSet.insert(uav.resource->BoundShaderType());
				}
			}
		}

		for (auto srItr : srUnbindSet)
		{
			std::vector<ResourceView> srs(_srsCache[srItr].size(), ResourceView());
			DoSetShaderResources(srItr, srs, 0);
			for (auto & sr : _srsCache[srItr])
			{
				if (sr.resource)
					sr.resource->ResetBind();
			}
			_srsCache[srItr].clear();
		}

		for (auto bufItr : bufUnbindSet)
		{
			std::vector<Ptr<RenderBuffer>> bufs(_bufsCache[bufItr].size(), Ptr<RenderBuffer>());
			DoSetShaderBuffers(bufItr, bufs, 0);
			for (auto & buf : _bufsCache[bufItr])
			{
				if (buf)
					buf->ResetBind();
			}
			_bufsCache[bufItr].clear();
		}
	}
}