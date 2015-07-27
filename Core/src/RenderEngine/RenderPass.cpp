#include "ToyGE\RenderEngine\RenderPass.h"
#include "ToyGE\RenderEngine\RenderTechnique.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderEffectVariable.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"

namespace ToyGE
{
	//RenderPass * RenderPass::_lastBindPass = nullptr;

	Ptr<RenderPass> RenderPass::Load(const rapidxml::xml_node<> * node, const Ptr<RenderTechnique> & technique)
	{
		if (nullptr == node)
			return Ptr<RenderPass>();

		static std::map<ShaderType, String> passShaderMap = 
		{
			{ SHADER_VS, "vs" },
			{ SHADER_GS, "gs" },
			{ SHADER_DS, "ds" },
			{ SHADER_HS, "hs" },
			{ SHADER_PS, "ps" },
			{ SHADER_CS, "cs" }
		};
		auto pass = std::make_shared<RenderPass>();
		pass->_technique = technique;
		////load macros
		//auto macroNode = node->first_node("macro");
		//while (macroNode)
		//{
		//	MacroDesc desc;
		//	desc.name = macroNode->first_attribute("name")->value();
		//	auto valueAttri = macroNode->first_attribute("value");
		//	if (valueAttri)
		//		desc.value = valueAttri->value();
		//	pass->_macros.push_back(desc);

		//	macroNode = macroNode->next_sibling("macro");
		//}

		//Load State
		auto stateNode = node->first_node("state");
		while (stateNode)
		{
			String valueStr = stateNode->first_attribute("value")->value();
			auto stateVar = pass->_technique.lock()->Effect().lock()->VariableByName(valueStr);
			if (stateVar)
			{
				if (stateVar->IsBlendState())
				{
					pass->_blendState = stateVar->AsBlendState()->GetBlendState();
					pass->_blendFactor = stateVar->AsBlendState()->BlendFactor();
					pass->_blendSampleMask = stateVar->AsBlendState()->BlendSampleMask();
				}
				else if (stateVar->IsDepthStencilState())
				{
					pass->_depthStencilState = stateVar->AsDepthStencilState()->GetDepthStencilState();
					pass->_stencilRef = stateVar->AsDepthStencilState()->StencilRef();
				}
				else if (stateVar->IsRasterizerState())
				{
					pass->_rasterizerState = stateVar->AsRasterizerState()->GetRasterizerState();
				}
				else
				{
					Logger::LogLine("effect error> pass state must be blendState/depthStencilState/rasterizerState [name:%s]", valueStr.c_str());
				}
			}
			else
			{
				Logger::LogLine("effect error> variable not found [name:%s]", valueStr.c_str());
			}

			stateNode = stateNode->next_sibling("state");
		}

		//Load Shaders
		for (auto itr = passShaderMap.begin(); itr != passShaderMap.end(); ++itr)
		{
			if (node->first_node(itr->second.c_str()))
			{
				String entry = node->first_node(itr->second.c_str())->first_attribute("entry")->value();
				pass->_shadersEntries[itr->first] = entry;
			}
		}

		return pass;
	}

	//void RenderPass::InitShaderContents()
	//{
	//	for (auto itr = _shadersEntries.begin(); itr != _shadersEntries.end(); ++itr)
	//	{
	//		auto shader = _technique.lock()->Effect().lock()->AcquireShader(itr->first, itr->second, _macros);
	//		if (shader)
	//		{
	//			_shaders[itr->first] = shader;

	//			auto & reflectDesc = shader->ReflectionInfo();

	//			//buffers
	//			if (reflectDesc.buffers.size() > 0)
	//			{
	//				int32_t numCBs = 0;
	//				for (auto & bufDesc : reflectDesc.buffers)
	//				{
	//					auto var = _technique.lock()->Effect().lock()->VariableByName(bufDesc.name);
	//					if (var->IsConstantBuffer())
	//						++numCBs;
	//				}

	//				shader->ResizeNumShaderBuffers(numCBs);

	//				int32_t bufIndex = 0;
	//				for (auto & bufDesc : reflectDesc.buffers)
	//				{
	//					auto var = _technique.lock()->Effect().lock()->VariableByName(bufDesc.name);
	//					if (var->IsConstantBuffer())
	//					{
	//						auto cbVar = var->AsConstantBuffer();
	//						shader->SetShaderBufferIndex(cbVar->Value(), bufIndex++);
	//					}
	//				}
	//			}

	//			//shader resources
	//			if (reflectDesc.shaderResources.size() > 0)
	//			{
	//				int32_t srMaxBindPoint = 0;
	//				for (auto & srDesc : reflectDesc.shaderResources)
	//				{
	//					srMaxBindPoint = std::max<int32_t>(srMaxBindPoint, srDesc.bindPoint);
	//				}
	//				shader->ResizeNumShaderResources(srMaxBindPoint + 1);
	//			}

	//			//uavs
	//			if (reflectDesc.uavs.size() > 0)
	//			{
	//				int32_t uavMinBindPoint = INT_MAX;
	//				int32_t uavMaxBindPoint = 0;
	//				for (auto & uavDesc : reflectDesc.uavs)
	//				{
	//					uavMinBindPoint = std::min<int32_t>(uavMinBindPoint, uavDesc.bindPoint);
	//					uavMaxBindPoint = std::max<int32_t>(uavMaxBindPoint, uavDesc.bindPoint);
	//				}
	//				shader->ResizeNumUAVs(uavMaxBindPoint - uavMinBindPoint + 1);
	//			}

	//			//samplers
	//			if (reflectDesc.samplers.size() > 0)
	//			{
	//				int32_t samMaxBindPoint = 0;
	//				for (auto & samDesc : reflectDesc.samplers)
	//				{
	//					samMaxBindPoint = std::max<int32_t>(samMaxBindPoint, samDesc.bindPoint);
	//				}
	//				shader->ResizeNumSamplers(samMaxBindPoint + 1);

	//				for (auto & samDesc : reflectDesc.samplers)
	//				{
	//					auto samVar = _technique.lock()->Effect().lock()->VariableByName(samDesc.name)->AsSampler();
	//					shader->SetSamplerIndex(samVar->Value(), samDesc.bindPoint);
	//				}
	//			}
	//		}
	//	}
	//}

	void RenderPass::Bind()
	{
		UpdateVariables();

		static std::vector<ShaderType> _sts =
		{
			SHADER_VS,
			SHADER_GS,
			SHADER_DS,
			SHADER_HS,
			SHADER_PS,
			SHADER_CS
		};

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		for (auto st : _sts)
		{
			auto shaderFind = _shaders.find(st);
			if (shaderFind == _shaders.end() || !shaderFind->second)
				rc->ResetShader(st);
			else
				rc->SetShader(shaderFind->second);
		}

		rc->SetBlendState(_blendState, _blendFactor, _blendSampleMask);
		rc->SetDepthStencilState(_depthStencilState, _stencilRef);
		rc->SetRasterizerState(_rasterizerState);
	}

	void RenderPass::UnBind()
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();
		if (_blendState)
			rc->SetBlendState(Ptr<BlendState>());
		if (_depthStencilState)
			rc->SetDepthStencilState(Ptr<DepthStencilState>());
		if (_rasterizerState)
			rc->SetRasterizerState(Ptr<RasterizerState>());
	}

	void RenderPass::UpdateVariables()
	{
		for (auto itr = _shaders.begin(); itr != _shaders.end(); ++itr)
		{
			if (!itr->second)
				continue;
			auto & shader = itr->second;
			auto & reflectDesc = shader->GetProgram()->GetReflectInfo();

			//Constant Buffers
			for (auto & bufDesc : reflectDesc.buffers)
			{
				auto bufVar = _technique.lock()->Effect().lock()->VariableByName(bufDesc.name)->AsConstantBuffer();

				if (bufVar->IsDirty())
				{
					if (!bufVar->IsShared())
					{
						auto bufMapData = reinterpret_cast<uint8_t*>(bufVar->GetValue()->Map(MAP_WRITE_DISCARD).pData);
						for (int32_t bufVarIndex = 0; bufVarIndex != bufVar->NumVariables(); ++bufVarIndex)
						{
							auto scalarVar = bufVar->VariableByIndex(bufVarIndex)->AsScalar();
							memcpy(bufMapData + scalarVar->BytesOffset(), scalarVar->Value(), scalarVar->Size());
						}
						bufVar->GetValue()->UnMap();
					}
					bufVar->SetDirty(false);
				}
				shader->SetConstantBuffer(bufDesc.bindPoint, bufVar->GetValue());
			}

			//Shader Resources
			for (auto & srDesc : reflectDesc.shaderResources)
			{
				auto srVar = _technique.lock()->Effect().lock()->VariableByName(srDesc.name)->AsShaderResource();
				if (srVar->Value() != shader->GetShaderResource(srDesc.bindPoint))
				{
					shader->SetShaderResource(srDesc.bindPoint, srVar->Value());
					srVar->SetDirty(false);
				}
			}

			//UAV
			int32_t minUAVBindPoint = INT_MAX;
			for (auto & uavDesc : reflectDesc.uavs)
			{
				minUAVBindPoint = std::min<int32_t>(minUAVBindPoint, uavDesc.bindPoint);
			}

			for (auto & uavDesc : reflectDesc.uavs)
			{
				auto uavVar = _technique.lock()->Effect().lock()->VariableByName(uavDesc.name)->AsUAV();
				if (uavVar->Value() != shader->GetUAV(uavDesc.bindPoint - minUAVBindPoint))
				{
					shader->SetUAV(uavDesc.bindPoint - minUAVBindPoint, uavVar->Value());
					uavVar->SetDirty(false);
				}
			}

			//Samplers
			for (auto & samDesc : reflectDesc.samplers)
			{
				auto samVar = _technique.lock()->Effect().lock()->VariableByName(samDesc.name)->AsSampler();
				shader->SetSampler(samDesc.bindPoint, samVar->Value());
			}
		}
	}
}