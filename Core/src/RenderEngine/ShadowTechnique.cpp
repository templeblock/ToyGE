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
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\RasterizerState.h"
#include "ToyGE\RenderEngine\LightComponent.h"

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
				/*auto & sm = _cameraRelevantShadowMap[sharedEnv->GetView()->GetCamera()];
				if (sm)
					sm->Release();*/
				for (auto & sm : _cameraRelevantShadowMap)
					sm.second->Release();
				_cameraRelevantShadowMap.clear();
			}
			for (auto & rsm : _rsm)
			{
				if (rsm)
				{
					rsm->Release();
					rsm = nullptr;
				}
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

			if (light->IsCastLPV())
			{
				texDesc.width = _rsmSize.x;
				texDesc.height = _rsmSize.y;

				//For Cascaded Shadow Map
				if(light->Type() == LIGHT_DIRECTIONAL)
					_rsm[0] = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

				texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
				_rsm[1] = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
				_rsm[2] = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
			}

			//Clear Shadow Map
			//auto & clearValue = _renderTechnique->GetShadowMapDefaultValue();
			float4 clearValue = 1.0f;
			if (texDesc.type == TEXTURE_CUBE)
			{
				rc->ClearRenderTargets({ shadowMap->CreateTextureView_Cube(0, 1, 0, texDesc.arraySize) }, 1.0f);
				for(auto & rsm : _rsm)
					if(rsm)
						rc->ClearRenderTargets({ rsm->CreateTextureView_Cube(0, 1, 0, texDesc.arraySize) }, 0.0f);
			}
			else
			{
				rc->ClearRenderTargets({ shadowMap->CreateTextureView(0, 1, 0, texDesc.arraySize) }, 1.0f);
				for (auto & rsm : _rsm)
					if(rsm)
						rc->ClearRenderTargets({ rsm->CreateTextureView(0, 1, 0, texDesc.arraySize) }, 0.0f);
			}

			//Render Depth
			_depthTechnique->RenderDepth(shadowMap, light, sharedEnv, _rsm);

			//Process
			auto processedSM = _renderTechnique->ProcessShadowTex(shadowMap, light);

			//Cache
			if (!IsRelevantWithCamera())
			{
				_shadowMap = processedSM;
				_rawShadowMap = shadowMap;
			}
			else
			{
				_cameraRelevantShadowMap[sharedEnv->GetView()->GetCamera()] = processedSM;
				_cameraRelevantRawShadowMap[sharedEnv->GetView()->GetCamera()] = shadowMap;
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

		Ptr<Texture> processedSM;
		Ptr<Texture> rawSM;
		if (!IsRelevantWithCamera())
		{
			processedSM = _shadowMap;
			rawSM = _rawShadowMap;
		}
		else
		{
			processedSM = _cameraRelevantShadowMap[camera];
			rawSM = _cameraRelevantRawShadowMap[camera];
		}

		auto & texDesc = processedSM->Desc();
		if (TEXTURE_2D == texDesc.type)
		{
			fx->VariableByName("shadowTexArray")->AsShaderResource()->SetValue(processedSM->CreateTextureView(0, 1, 0, texDesc.arraySize));
			fx->VariableByName("rawShadowTexArray")->AsShaderResource()->SetValue(rawSM->CreateTextureView(0, 1, 0, texDesc.arraySize));
		}
		else if (TEXTURE_CUBE == texDesc.type)
		{
			fx->VariableByName("shadowTexCubeArray")->AsShaderResource()->SetValue(processedSM->CreateTextureView_Cube(0, 1, 0, texDesc.arraySize));
			fx->VariableByName("rawShadowTexCubeArray")->AsShaderResource()->SetValue(rawSM->CreateTextureView_Cube(0, 1, 0, texDesc.arraySize));
			fx->VariableByName("shadowTexArray")->AsShaderResource()->SetValue(processedSM->CreateTextureView(0, 1, 0, texDesc.arraySize * 6));
			fx->VariableByName("rawShadowTexArray")->AsShaderResource()->SetValue(rawSM->CreateTextureView(0, 1, 0, texDesc.arraySize * 6));
		}

		_renderTechnique->BindParams(fx, light, processedSM);
	}


	ShadowDepthTechnique::ShadowDepthTechnique()
		: _depthBias(0),
		_slopedScaledDepthBias(0.0f)
	{
		RasterizerStateDesc rsDesc;
		rsDesc.depthBias = _depthBias;
		rsDesc.slopeScaledDepthBias = _slopedScaledDepthBias;

		_depthRenderRS = Global::GetRenderEngine()->GetRenderFactory()->CreateRasterizerState(rsDesc);
	}

	void ShadowDepthTechnique::UpdateDepthRasterizerState()
	{
		if (_depthRenderRS->Desc().depthBias != _depthBias
			|| _depthRenderRS->Desc().slopeScaledDepthBias != _slopedScaledDepthBias)
		{
			RasterizerStateDesc rsDesc;
			rsDesc.depthBias = _depthBias;
			rsDesc.slopeScaledDepthBias = _slopedScaledDepthBias;
			_depthRenderRS = Global::GetRenderEngine()->GetRenderFactory()->CreateRasterizerState(rsDesc);
		}
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