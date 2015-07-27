#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"

namespace ToyGE
{
	void Shader::SetProgram(const Ptr<ShaderProgram> & program)
	{
		_shaderProgram = program;

		auto & reflectInfo = _shaderProgram->GetReflectInfo();

		//Constant Buffers
		_cbs.clear();
		if (reflectInfo.buffers.size() > 0)
		{
			int32_t cbsMaxBindPoint = 0;
			for (auto & bufDesc : reflectInfo.buffers)
			{
				cbsMaxBindPoint = std::max<int32_t>(cbsMaxBindPoint, bufDesc.bindPoint);
			}
			_cbs.resize(cbsMaxBindPoint + 1);
		}

		//Shader Resources
		_srs.clear();
		if (reflectInfo.shaderResources.size() > 0)
		{
			int32_t srMaxBindPoint = 0;
			for (auto & srDesc : reflectInfo.shaderResources)
			{
				srMaxBindPoint = std::max<int32_t>(srMaxBindPoint, srDesc.bindPoint);
			}
			_srs.resize(srMaxBindPoint + 1);
		}

		//UAVs
		_uavs.clear();
		if (reflectInfo.uavs.size() > 0)
		{
			int32_t uavMinBindPoint = INT_MAX;
			int32_t uavMaxBindPoint = 0;
			for (auto & uavDesc : reflectInfo.uavs)
			{
				uavMinBindPoint = std::min<int32_t>(uavMinBindPoint, uavDesc.bindPoint);
				uavMaxBindPoint = std::max<int32_t>(uavMaxBindPoint, uavDesc.bindPoint);
			}
			_uavs.resize(uavMaxBindPoint - uavMinBindPoint + 1);
		}

		//Samplers
		_samplers.clear();
		if (reflectInfo.samplers.size() > 0)
		{
			int32_t samMaxBindPoint = 0;
			for (auto & samDesc : reflectInfo.samplers)
			{
				samMaxBindPoint = std::max<int32_t>(samMaxBindPoint, samDesc.bindPoint);
			}
			_samplers.resize(samMaxBindPoint + 1);

			/*for (auto & samDesc : reflectInfo.samplers)
			{
				auto samVar = _technique.lock()->Effect().lock()->VariableByName(samDesc.name)->AsSampler();
				shader->SetSamplerIndex(samVar->Value(), samDesc.bindPoint);
			}*/
		}
	}
}