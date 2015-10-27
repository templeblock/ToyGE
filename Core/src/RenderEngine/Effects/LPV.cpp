#include "ToyGE\RenderEngine\Effects\LPV.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	namespace
	{
		struct VPL
		{
			float3 normal;
			float3 flux;
			int32_t next;
		};

		static const int3 lpvGridDims = int3(32, 32, 32);
	}

	LPV::LPV()
		: _lpvStrength(1.0f),
		_bGeometryOcclusion(true)
	{
		static const int32_t maxVPLs = 256 * 256;

		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_STRUCTURED | BUFFER_BIND_UNORDERED_ACCESS;
		bufDesc.cpuAccess = 0;
		bufDesc.elementSize = sizeof(VPL);
		bufDesc.numElements = maxVPLs;
		bufDesc.structedByteStride = bufDesc.elementSize;
		_vplListBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(bufDesc, nullptr);

		bufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_UNORDERED_ACCESS | BUFFER_BIND_RAW;
		bufDesc.elementSize = sizeof(int32_t);
		bufDesc.numElements = (lpvGridDims.x * lpvGridDims.y * lpvGridDims.z);
		_vplHeadBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(bufDesc, nullptr);

		TextureDesc texDesc;
		texDesc.width = lpvGridDims.x;
		texDesc.height = lpvGridDims.y;
		texDesc.depth = lpvGridDims.z;
		texDesc.mipLevels = 1;
		texDesc.arraySize = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_3D;
		for (auto & volume : _lightVolumes)
			volume = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		for (auto & volume : _lightVolumesBack)
			volume = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		
		_geometryVolumes[0] = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		_geometryVolumes[1] = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		texDesc.format = RENDER_FORMAT_R16_FLOAT;
		_geometryVolumes[2] = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"LPV.xml");
		_renderFx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"LPVIndirectRender.xml");
	}

	void LPV::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto sceneAABB = Global::GetRenderEngine()->GetSceneRenderObjsCuller()->GetSceneAABB();
		float3 sceneMin, sceneMax;
		Math::AxisAlignedBoxToMinMax(sceneAABB, sceneMin, sceneMax);

		_worldToGridScale =
			float3(static_cast<float>(lpvGridDims.x), static_cast<float>(lpvGridDims.y), static_cast<float>(lpvGridDims.z))
			/ (sceneMax - sceneMin);
		_worldToGridOffset = sceneMin * _worldToGridScale * -1.0f;


		ClearVPL();

		std::vector<Ptr<Cullable>> cullLights;
		Global::GetRenderEngine()->GetSceneRenderLightsCuller()->GetAllElements(cullLights);
		for (auto & cull : cullLights)
		{
			auto light = std::static_pointer_cast<LightComponent>(cull);
			if (light->IsCastLPV())
			{
				light->BindMacros(_fx, true, nullptr);
				_fx->UpdateData();
				light->BindParams(_fx, true, nullptr);

				GenerateVPLList(light);
				if(_bGeometryOcclusion)
					BuildGeometryVolume();
				Inject();
				break;
			}
		}

		Propagate();

		auto linearDepthTex = sharedEnviroment->GetTextureParam(CommonRenderShareName::LinearDepth());
		auto gbuffer0 = sharedEnviroment->GetTextureParam(CommonRenderShareName::GBuffer(0));
		auto gbuffer1 = sharedEnviroment->GetTextureParam(CommonRenderShareName::GBuffer(1));
		auto gbuffer2 = sharedEnviroment->GetTextureParam(CommonRenderShareName::GBuffer(2));

		sharedEnviroment->GetView()->BindParams(_renderFx);

		auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(sharedEnviroment->GetView()->GetRenderResult()->Desc());

		RenderIndirect(linearDepthTex, gbuffer0, gbuffer1, gbuffer2, sharedEnviroment->GetView()->GetRenderResult());
		//RenderIndirect(linearDepthTex, gbuffer0, gbuffer1, gbuffer2, tmpTex);

		tmpTex->Release();
	}

	void LPV::ClearVPL()
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		_fx->VariableByName("vplHeadBufferRW")->AsUAV()->SetValue(_vplHeadBuffer->CreateBufferView(RENDER_FORMAT_R32_TYPELESS, 0, 0, BUFFER_UAV_RAW));

		auto numGrids = _vplHeadBuffer->Desc().numElements;
		auto numGroups = (numGrids + 63) / 64;

		_fx->TechniqueByName("ClearVPLs")->PassByIndex(0)->Bind();
		rc->Compute(numGroups, 1, 1);
		_fx->TechniqueByName("ClearVPLs")->PassByIndex(0)->UnBind();
	}

	void LPV::GenerateVPLList(const Ptr<LightComponent> & light)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto & rsm = light->GetShadowTechnique()->GetRSM();

		_fx->VariableByName("worldToGridScale")->AsScalar()->SetValue(&_worldToGridScale);
		_fx->VariableByName("worldToGridOffset")->AsScalar()->SetValue(&_worldToGridOffset);
		_fx->VariableByName("lpvGridDims")->AsScalar()->SetValue(&lpvGridDims);

		light->GetShadowTechnique()->DepthTechnique()->BindRSMParams(_fx);

		_fx->VariableByName("rsmTexSize")->AsScalar()->SetValue(&rsm[0]->GetTexSize());

		_fx->VariableByName("rsmDepth")->AsShaderResource()->SetValue(rsm[0]->CreateTextureView());
		_fx->VariableByName("rsmNormal")->AsShaderResource()->SetValue(rsm[1]->CreateTextureView());
		_fx->VariableByName("rsmDiffuseColor")->AsShaderResource()->SetValue(rsm[2]->CreateTextureView());
		_fx->VariableByName("vplListBufferRW")->AsUAV()->SetValue(_vplListBuffer->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, 0, BUFFER_UAV_COUNTER));
		_fx->VariableByName("vplHeadBufferRW")->AsUAV()->SetValue(_vplHeadBuffer->CreateBufferView(RENDER_FORMAT_R32_TYPELESS, 0, 0, BUFFER_UAV_RAW));

		_fx->TechniqueByName("GenerateVPLs")->PassByIndex(0)->Bind();
		rc->Compute((rsm[0]->Desc().width + 7) / 8, (rsm[0]->Desc().height + 7) / 8, 1);
		_fx->TechniqueByName("GenerateVPLs")->PassByIndex(0)->UnBind();
	}

	void LPV::BuildGeometryVolume()
	{
		_fx->VariableByName("vplListBuffer")->AsShaderResource()->SetValue(_vplListBuffer->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, 0));
		_fx->VariableByName("vplHeadBuffer")->AsShaderResource()->SetValue(_vplHeadBuffer->CreateBufferView(RENDER_FORMAT_R32_TYPELESS, 0, 0));

		auto depth = _geometryVolumes[0]->Desc().depth;
		for (int i = 0; i < _geometryVolumes.size(); ++i)
			_fx->VariableByName("geometryVolumeRW" + std::to_string(i))->AsUAV()->SetValue(_geometryVolumes[i]->CreateTextureView(0, 1, 0, depth));

		int groupX = (_geometryVolumes[0]->Desc().width + 3) / 4;
		int groupY = (_geometryVolumes[0]->Desc().height + 3) / 4;
		int groupZ = (_geometryVolumes[0]->Desc().depth + 3) / 4;

		_fx->TechniqueByName("GVInject")->PassByIndex(0)->Bind();
		Global::GetRenderEngine()->GetRenderContext()->Compute(groupX, groupY, groupZ);
		_fx->TechniqueByName("GVInject")->PassByIndex(0)->UnBind();
	}

	void LPV::Inject()
	{
		_fx->VariableByName("vplListBuffer")->AsShaderResource()->SetValue(_vplListBuffer->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, 0));
		_fx->VariableByName("vplHeadBuffer")->AsShaderResource()->SetValue(_vplHeadBuffer->CreateBufferView(RENDER_FORMAT_R32_TYPELESS, 0, 0));

		auto depth = _lightVolumes[0]->Desc().depth;
		for (int i = 0; i < _lightVolumes.size(); ++i)
			_fx->VariableByName("lightVolumeRW" + std::to_string(i))->AsUAV()->SetValue(_lightVolumes[i]->CreateTextureView(0, 1, 0, depth));

		int groupX = (_lightVolumes[0]->Desc().width + 3) / 4;
		int groupY = (_lightVolumes[0]->Desc().height + 3) / 4;
		int groupZ = (_lightVolumes[0]->Desc().depth + 3) / 4;

		_fx->TechniqueByName("LPVInject")->PassByIndex(0)->Bind();
		Global::GetRenderEngine()->GetRenderContext()->Compute(groupX, groupY, groupZ);
		_fx->TechniqueByName("LPVInject")->PassByIndex(0)->UnBind();
	}

	void LPV::Propagate()
	{
		if (_bGeometryOcclusion)
			_fx->AddExtraMacro("GEOMETRY_OCCLUSION", "");
		else
			_fx->RemoveExtraMacro("GEOMETRY_OCCLUSION");
		_fx->UpdateData();

		for (int i = 0; i < _numPropagationItrs; ++i)
		{
			auto depth = _lightVolumes[0]->Desc().depth;

			for (int i = 0; i < _lightVolumes.size(); ++i)
				_fx->VariableByName("lightVolume" + std::to_string(i))->AsShaderResource()->SetValue(_lightVolumes[i]->CreateTextureView(0, 1, 0, depth));
			for (int i = 0; i < _lightVolumesBack.size(); ++i)
				_fx->VariableByName("lightVolumeRW" + std::to_string(i))->AsUAV()->SetValue(_lightVolumesBack[i]->CreateTextureView(0, 1, 0, depth));

			for (int i = 0; i < _geometryVolumes.size(); ++i)
				_fx->VariableByName("geometryVolume" + std::to_string(i))->AsShaderResource()->SetValue(_geometryVolumes[i]->CreateTextureView(0, 1, 0, depth));

			int groupX = (_lightVolumes[0]->Desc().width + 3) / 4;
			int groupY = (_lightVolumes[0]->Desc().height + 3) / 4;
			int groupZ = (_lightVolumes[0]->Desc().depth + 3) / 4;

			_fx->TechniqueByName("LPVPropagate")->PassByIndex(0)->Bind();
			Global::GetRenderEngine()->GetRenderContext()->Compute(groupX, groupY, groupZ);
			_fx->TechniqueByName("LPVPropagate")->PassByIndex(0)->UnBind();

			for (int i = 0; i < _lightVolumes.size(); ++i)
				std::swap(_lightVolumes[i], _lightVolumesBack[i]);
		}
	}

	void LPV::RenderIndirect(
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & gbuffer0,
		const Ptr<Texture> & gbuffer1,
		const Ptr<Texture> & gbuffer2,
		const Ptr<Texture> & targetTex)
	{
		_renderFx->VariableByName("lpvStrength")->AsScalar()->SetValue(&_lpvStrength);

		_renderFx->VariableByName("worldToGridScale")->AsScalar()->SetValue(&_worldToGridScale);
		_renderFx->VariableByName("worldToGridOffset")->AsScalar()->SetValue(&_worldToGridOffset);
		_renderFx->VariableByName("lpvGridDims")->AsScalar()->SetValue(&lpvGridDims);

		auto depth = _lightVolumes[0]->Desc().depth;

		for (int i = 0; i < _lightVolumes.size(); ++i)
			_renderFx->VariableByName("lightVolume" + std::to_string(i))->AsShaderResource()->SetValue(_lightVolumes[i]->CreateTextureView(0, 1, 0, depth));

		_renderFx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());
		_renderFx->VariableByName("gbuffer0")->AsShaderResource()->SetValue(gbuffer0->CreateTextureView());
		_renderFx->VariableByName("gbuffer1")->AsShaderResource()->SetValue(gbuffer1->CreateTextureView());
		_renderFx->VariableByName("gbuffer2")->AsShaderResource()->SetValue(gbuffer2->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ targetTex->CreateTextureView() }, 0);

		RenderQuad(_renderFx->TechniqueByName("RenderLPVIndirect"));
	}
}