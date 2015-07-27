#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	bool ShadowTechnique::IsRelevantWithCamera() const
	{
		if (_depthTechnique)
			return _depthTechnique->IsRelevantWithCamera();

		return false;
	}

	void ShadowTechnique::PrepareShadow(const Ptr<LightComponent> & light, const Ptr<RenderSharedEnviroment> & sharedEnv)
	{
		if (_depthTechnique && _renderTechnique)
		{
			auto rc = Global::GetRenderEngine()->GetRenderContext();

			//Release
			if (!IsRelevantWithCamera())
			{
				if (_shadowMap)
					_shadowMap->Release();
			}
			else
			{
				auto & sm = _cameraRelevantShadowMap[sharedEnv->GetView()->GetCamera()];
				if (sm)
					sm->Release();
			}

			//Get Shadow Map Texture
			TextureDesc texDesc;
			texDesc.width = _shadowMapSize.x;
			texDesc.height = _shadowMapSize.y;
			texDesc.depth = 1;
			texDesc.mipLevels = 1;
			texDesc.arraySize = _depthTechnique->GetShadowMapArraySize();
			texDesc.type = _depthTechnique->GetShadowMapTextureType();
			texDesc.format = RENDER_FORMAT_R32_FLOAT;
			texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
			texDesc.cpuAccess = 0;
			texDesc.sampleCount = 1;
			texDesc.sampleQuality = 0;
			auto shadowMap = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

			//Clear Shadow Map
			//auto & clearValue = _renderTechnique->GetShadowMapDefaultValue();
			float4 clearValue = 1.0f;
			if (texDesc.type == TEXTURE_CUBE)
			{
				rc->ClearRenderTargets({ shadowMap->CreateTextureView_Cube(0, 1, 0, texDesc.arraySize) }, 1.0f);
			}
			else
			{
				rc->ClearRenderTargets({ shadowMap->CreateTextureView(0, 1, 0, texDesc.arraySize) }, 1.0f);
			}

			//Render Depth
			_depthTechnique->RenderDepth(shadowMap, light, sharedEnv);

			//Process
			shadowMap = _renderTechnique->ProcessShadowTex(shadowMap, light);

			//Cache
			if (!IsRelevantWithCamera())
			{
				_shadowMap = shadowMap;
			}
			else
			{
				_cameraRelevantShadowMap[sharedEnv->GetView()->GetCamera()] = shadowMap;
			}
		}
	}

	void ShadowTechnique::BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Camera> & camera)
	{
		_depthTechnique->BindMacros(fx, light);

		Ptr<Texture> sm;
		if (!IsRelevantWithCamera())
		{
			sm = _shadowMap;
		}
		else
		{
			sm = _cameraRelevantShadowMap[camera];
		}
		_renderTechnique->BindMacros(fx, light, sm);
	}

	void ShadowTechnique::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Camera> & camera)
	{
		_depthTechnique->BindParams(fx, light);

		Ptr<Texture> sm;
		if (!IsRelevantWithCamera())
		{
			sm = _shadowMap;
		}
		else
		{
			sm = _cameraRelevantShadowMap[camera];
		}
		_renderTechnique->BindParams(fx, light, sm);
	}



	void ShadowRenderTechnique::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		auto & texDesc = shadowMap->Desc();
		if (TEXTURE_2D == texDesc.type)
		{
			fx->VariableByName("shadowTexArray")->AsShaderResource()->SetValue(shadowMap->CreateTextureView(0, 1, 0, texDesc.arraySize));
		}
		else if (TEXTURE_CUBE == texDesc.type)
		{
			fx->VariableByName("shadowTexCubeArray")->AsShaderResource()->SetValue(shadowMap->CreateTextureView_Cube(0, 1, 0, texDesc.arraySize));
		}
	}

}