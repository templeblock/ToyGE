#include "ToyGE\RenderEngine\ReflectionMap.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	Ptr<Texture> ReflectionMap::_lut;
	int32_t ReflectionMap::_lutSize = 512;

	void ReflectionMap::InitLUT()
	{
		_lut = nullptr;

		TextureDesc texDesc;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.width = _lutSize;
		texDesc.height = _lutSize;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.cpuAccess = 0;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		texDesc.mipLevels = 1;
		texDesc.format = RENDER_FORMAT_R16G16_FLOAT;
		_lut = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_lut->SetDesc(texDesc);
		_lut->Init();

		auto ps = Shader::FindOrCreate<PreComputedLUTPS>();
		ps->SetScalar("texSize", _lut->GetTexSize());
		ps->Flush();

		DrawQuad({ _lut->GetRenderTargetView(0, 0, 1) });
	}

	ReflectionMap::ReflectionMap()
	{
		_preComputedMapSize = 512;
	}

	void ReflectionMap::InitPreComputedData()
	{
		_prefiltedEnviromentMapRef = nullptr;

		TextureDesc texDesc;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.width = _preComputedMapSize;
		texDesc.height = _preComputedMapSize;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.cpuAccess = 0;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.bCube = true;

		texDesc.mipLevels = 0;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		_prefiltedEnviromentMapRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto prefiltedEnviromentMap = _prefiltedEnviromentMapRef->Get()->Cast<Texture>();

		{
			auto ps = Shader::FindOrCreate<PrefilterEnvMapPS>();

			auto numMips = prefiltedEnviromentMap->GetDesc().mipLevels;
			for (int32_t mipLevel = 0; mipLevel < numMips; ++mipLevel)
			{
				for (int32_t cubeFace = 0; cubeFace < 6; ++cubeFace)
				{
					auto mipSize = prefiltedEnviromentMap->GetMipSize(mipLevel);

					float4 texSize = float4((float)mipSize.x(), (float)mipSize.y(), 1.0f / (float)mipSize.x(), 1.0f / (float)mipSize.y());

					ps->SetScalar("arrayIndex", cubeFace);
					ps->SetScalar("texSize", texSize);
					ps->SetScalar("mipCoord", (float)mipLevel / (float)(numMips - 1));

					ps->SetSRV("envMap", _enviromentMap->GetShaderResourceView(0, 0, 0, 0, true));

					ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

					ps->Flush();

					DrawQuad({ prefiltedEnviromentMap->GetRenderTargetView(mipLevel, cubeFace, 1) });
				}
			}
		}
	}

	void ReflectionMap::BindShaderParams(const Ptr<Shader> & shader)
	{
		float4 iblLUTSize = float4((float)_lutSize, (float)_lutSize, 1.0f / (float)_lutSize, 1.0f / (float)_lutSize);
		shader->SetScalar("iblLUTSize", iblLUTSize);
		shader->SetScalar("numEnvMapMipLevels", (float)_prefiltedEnviromentMapRef->Get()->Cast<Texture>()->GetDesc().mipLevels);

		shader->SetSRV("iblLUT", _lut->GetShaderResourceView());
		shader->SetSRV("iblPreFilteredEnvMap", _prefiltedEnviromentMapRef->Get()->Cast<Texture>()->GetShaderResourceView(0, 0, 0, 0, true));

		shader->SetSampler("iblSampler", SamplerTemplate<>::Get());
	}
}