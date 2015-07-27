#include "ToyGE\RenderEngine\ReflectionMap.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\RenderInput.h"

namespace ToyGE
{
	int32_t ReflectionMap::_defaultMapSize = 1024;
	int32_t ReflectionMap::_idAll = -1;

	std::map<int32_t, Ptr<ReflectionMap>> ReflectionMap::_reflectionMaps;

	Ptr<ReflectionMap> ReflectionMap::Create()
	{
		auto pMap = new ReflectionMap();
		auto result = std::shared_ptr<ReflectionMap>(pMap);
		_reflectionMaps[result->_id] = result;
		return result;
	}

	Ptr<ReflectionMap> ReflectionMap::GetReflectionMap(int32_t id)
	{
		auto mapFind = _reflectionMaps.find(id);
		return mapFind == _reflectionMaps.end() ? Ptr<ReflectionMap>() : mapFind->second;
	}

	void ReflectionMap::RemoveReflectionMap(int32_t id)
	{
		_reflectionMaps.erase(_reflectionMaps.find(id));
	}

	void ReflectionMap::Clear()
	{
		_reflectionMaps.clear();
	}

	ReflectionMap::ReflectionMap()
		: _mapSize(_defaultMapSize),
		_id(++_idAll)
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"IBLPreCompute.xml");
		InitMapTextures();
	}

	void ReflectionMap::InitPreComputedData()
	{
		if (!_envMap)
			return;

		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		RenderContextStateSave save;
		rc->SaveState(
			RENDER_CONTEXT_STATE_DEPTHSTENCIL
			| RENDER_CONTEXT_STATE_RENDERTARGETS
			| RENDER_CONTEXT_STATE_VIEWPORT
			| RENDER_CONTEXT_STATE_INPUT, save);

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		//PrefilterEnvMap
		auto numMips = _prefiltedEnvMap->Desc().mipLevels;
		int32_t width = _prefiltedEnvMap->Desc().width;
		int32_t height = _prefiltedEnvMap->Desc().height;
		for (int32_t mipLevel = 0; mipLevel < numMips; ++mipLevel)
		{
			float4 textureSize;
			textureSize.x = static_cast<float>(width);
			textureSize.y = static_cast<float>(height);
			textureSize.z = static_cast<float>(mipLevel) / static_cast<float>(numMips - 1);

			vp.width = textureSize.x;
			vp.height = textureSize.y;
			rc->SetViewport(vp);

			_fx->VariableByName("textureSize")->AsScalar()->SetValue(&textureSize);
			_fx->VariableByName("envMap")->AsShaderResource()->SetValue(_envMap->CreateTextureView_Cube(0, 1, 0, 1));

			rc->SetRenderTargets({ _prefiltedEnvMap->CreateTextureView(mipLevel, 1, 0, 6) }, 0);
			_fx->TechniqueByName("PrefilterEnvMap")->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			_fx->TechniqueByName("PrefilterEnvMap")->PassByIndex(0)->UnBind();

			width /= 2;
			height /= 2;
		}

		//LUT
		vp.width = static_cast<float>(_LUT->Desc().width);
		vp.height = static_cast<float>(_LUT->Desc().height);
		rc->SetViewport(vp);

		float4 textureSize = 0.0f;
		textureSize.x = vp.width;
		textureSize.y = vp.height;
		_fx->VariableByName("textureSize")->AsScalar()->SetValue(&textureSize);

		rc->SetRenderTargets({ _LUT->CreateTextureView() }, 0);
		_fx->TechniqueByName("LUT")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("LUT")->PassByIndex(0)->UnBind();

		rc->RestoreState(save);
	}

	void ReflectionMap::BindEffectParams(const Ptr<RenderEffect> & fx)
	{
		fx->VariableByName("prefiltedEnvMap")->AsShaderResource()->SetValue(_prefiltedEnvMap->CreateTextureView_Cube(0, 0, 0, 1));
		fx->VariableByName("LUT")->AsShaderResource()->SetValue(_LUT->CreateTextureView());
		float numMipLevels = static_cast<float>(_prefiltedEnvMap->Desc().mipLevels);
		fx->VariableByName("numMipLevels")->AsScalar()->SetValue(&numMipLevels, sizeof(numMipLevels));
	}

	void ReflectionMap::InitMapTextures()
	{
		TextureDesc desc;
		desc.width = _mapSize;
		desc.height = _mapSize;
		desc.depth = 1;
		desc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		desc.cpuAccess = 0;
		desc.sampleCount = 1;
		desc.sampleQuality = 0;
		desc.arraySize = 1;

		desc.mipLevels = 0;
		desc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		desc.type = TEXTURE_CUBE;
		_prefiltedEnvMap = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(desc);

		desc.mipLevels = 1;
		desc.format = RENDER_FORMAT_R16G16_FLOAT;
		desc.type = TEXTURE_2D;
		_LUT = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(desc);
	}
}