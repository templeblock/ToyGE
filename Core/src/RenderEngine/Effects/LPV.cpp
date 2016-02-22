#include "ToyGE\RenderEngine\Effects\LPV.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\RenderEngine\Blur.h"
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

		static const int3 lpvGridDims = int3(16, 16, 16);
	}

	LPV::LPV()
		: _lpvStrength(1.0f),
		_numPropagationItrs(8),
		_bGeometryOcclusion(true)
	{
	}

	void LPV::Render(const Ptr<RenderView> & view)
	{
		static const int32_t maxVPLs = 256 * 256;

		RenderBufferDesc bufDesc;
		bufDesc.cpuAccess = 0;

		bufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_UNORDERED_ACCESS | BUFFER_BIND_RAW;
		bufDesc.elementSize = sizeof(int32_t);
		bufDesc.numElements = (lpvGridDims.x() * lpvGridDims.y() * lpvGridDims.z());
		//bufDesc.bStructured = true;
		auto vplHeadBufferRef = BufferPool::Instance().FindFree(bufDesc);
		auto vplHeadBuffer = vplHeadBufferRef->Get()->Cast<RenderBuffer>();

		bufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_UNORDERED_ACCESS;
		bufDesc.elementSize = sizeof(VPL);
		bufDesc.numElements = maxVPLs;
		bufDesc.bStructured = true;
		auto vplListBufferRef = BufferPool::Instance().FindFree(bufDesc);
		auto vplListBuffer = vplListBufferRef->Get()->Cast<RenderBuffer>();

		auto sceneAABB = Global::GetRenderEngine()->GetSceneRenderObjsCuller()->GetSceneAABB();
		//float3 sceneMin, sceneMax;
		//Math::AxisAlignedBoxToMinMax(sceneAABB, sceneMin, sceneMax);

		auto worldToGridScale =
			float3(static_cast<float>(lpvGridDims.x()), static_cast<float>(lpvGridDims.y()), static_cast<float>(lpvGridDims.z()))
			/ (sceneAABB.max - sceneAABB.min);
		auto worldToGridOffset = sceneAABB.min * -1.0f;

		ClearVPL(vplHeadBuffer);

		std::array<PooledTextureRef, 7> lightVolumes;
		std::array<PooledTextureRef, 3> geometryVolumes;

		TextureDesc texDesc;
		texDesc.width = lpvGridDims.x();
		texDesc.height = lpvGridDims.y();
		texDesc.depth = lpvGridDims.z();
		texDesc.mipLevels = 1;
		texDesc.arraySize = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		for (auto & volume : lightVolumes)
			volume = TexturePool::Instance().FindFree({ TEXTURE_3D, texDesc });

		geometryVolumes[0] = TexturePool::Instance().FindFree({ TEXTURE_3D, texDesc });
		geometryVolumes[1] = TexturePool::Instance().FindFree({ TEXTURE_3D, texDesc });
		texDesc.format = RENDER_FORMAT_R16_FLOAT;
		geometryVolumes[2] = TexturePool::Instance().FindFree({ TEXTURE_3D, texDesc });

		std::vector<Ptr<Cullable>> cullLights;
		Global::GetRenderEngine()->GetSceneRenderLightsCuller()->GetAllElements(cullLights);
		for (auto & cull : cullLights)
		{
			auto light = std::static_pointer_cast<LightComponent>(cull);
			if (light->IsCastLPV())
			{
				GenerateVPLList(light, worldToGridScale, worldToGridOffset, vplHeadBuffer, vplListBuffer);

				if (_bGeometryOcclusion)
					BuildGeometryVolume(light, vplHeadBuffer, vplListBuffer, geometryVolumes);

				Inject(light, vplHeadBuffer, vplListBuffer, lightVolumes);

				break;
			}
		}

		Propagate(geometryVolumes, lightVolumes);

		auto linearDepthTex = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
		auto gbuffer0 = view->GetViewRenderContext()->GetSharedTexture("GBuffer0");
		auto gbuffer1 = view->GetViewRenderContext()->GetSharedTexture("GBuffer1");
		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		RenderIndirect(
			view, 
			worldToGridScale, 
			worldToGridOffset, 
			lightVolumes, 
			gbuffer0, 
			gbuffer1, 
			linearDepthTex, 
			sceneTex->GetRenderTargetView(0, 0, 1));
	}

	void LPV::ClearVPL(const Ptr<RenderBuffer> & vplHeadBuffer)
	{
		auto cs = Shader::FindOrCreate<ClearVPLsCS>();

		cs->SetUAV("vplHeadBufferRW", vplHeadBuffer->GetUnorderedAccessView(0, 0, RENDER_FORMAT_R32_TYPELESS, BUFFER_UAV_RAW));

		cs->Flush();

		auto numGrids = vplHeadBuffer->GetDesc().numElements;
		auto numGroups = (numGrids + 63) / 64;

		Global::GetRenderEngine()->GetRenderContext()->Compute(numGroups, 1, 1);

		Global::GetRenderEngine()->GetRenderContext()->ResetShader(SHADER_CS);
	}

	void LPV::GenerateVPLList(
		const Ptr<LightComponent> & light,
		const float3 & worldToGridScale,
		const float3 & worldToGridOffset,
		const Ptr<RenderBuffer> & vplHeadBuffer,
		const Ptr<RenderBuffer> & vplListBuffer)
	{
		RenderBufferDesc bufDesc;
		bufDesc.cpuAccess = 0;
		bufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_UNORDERED_ACCESS;
		bufDesc.elementSize = sizeof(float);
		bufDesc.numElements = 1;
		bufDesc.bStructured = true;
		auto vplCounterRef = BufferPool::Instance().FindFree(bufDesc);
		auto vplCounter = vplCounterRef->Get()->Cast<RenderBuffer>();

		auto cs = Shader::FindOrCreate<GenerateVPLsCS>();

		cs->SetScalar("worldToGridScale", worldToGridScale);
		cs->SetScalar("worldToGridOffset", worldToGridOffset);
		cs->SetScalar("lpvGridDims", lpvGridDims);

		auto & rsm = light->GetShadowTechnique()->GetRSM();

		cs->SetScalar("rsmTexSize", rsm.rsmRadiance->Get()->Cast<Texture>()->GetTexSize());

		auto rsmWorldToClip = light->GetShadowTechnique()->Cast<CascadedShadow>()->GetRSMView().shadowWorldToClipMatrix[0];
		auto rsmClipToWorld = inverse(rsmWorldToClip);
		/*XMFLOAT4X4 rsmClipToWorld;
		XMStoreFloat4x4(&rsmClipToWorld, rsmClipToWorldXM);*/
		cs->SetScalar("rsmClipToWorld", rsmClipToWorld);

		cs->SetUAV("vplCounter", vplCounter->GetUnorderedAccessView(0, 0, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_COUNTER), 0);
		cs->SetUAV("vplHeadBufferRW", vplHeadBuffer->GetUnorderedAccessView(0, 0, RENDER_FORMAT_R32_TYPELESS, BUFFER_UAV_RAW));
		cs->SetUAV("vplListBufferRW", vplListBuffer->GetUnorderedAccessView(0, 0, RENDER_FORMAT_UNKNOWN, 0));

		cs->SetSRV("rsmRadiance", rsm.rsmRadiance->Get()->Cast<Texture>()->GetShaderResourceView());
		cs->SetSRV("rsmDepth", rsm.rsmDepth->Get()->Cast<Texture>()->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R32_FLOAT));
		cs->SetSRV("rsmNormal", rsm.rsmNormal->Get()->Cast<Texture>()->GetShaderResourceView());

		cs->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->Compute(
			(rsm.rsmRadiance->Get()->Cast<Texture>()->GetDesc().width + 7) / 8, 
			(rsm.rsmRadiance->Get()->Cast<Texture>()->GetDesc().height + 7) / 8,
			1);

		rc->ResetShader(SHADER_CS);
	}

	void LPV::BuildGeometryVolume(
		const Ptr<LightComponent> & light,
		const Ptr<RenderBuffer> & vplHeadBuffer,
		const Ptr<RenderBuffer> & vplListBuffer,
		const std::array<PooledTextureRef, 3> & geometryVolumes)
	{
		std::map<String, String> macros;
		light->BindMacros(false, nullptr, macros);

		auto cs = Shader::FindOrCreate<GVInjectCS>(macros);

		light->BindShaderParams(cs, false, nullptr);

		cs->SetScalar("lpvGridDims", lpvGridDims);

		cs->SetSRV("vplHeadBuffer", vplHeadBuffer->GetShaderResourceView(0, 0, RENDER_FORMAT_R32_TYPELESS));
		cs->SetSRV("vplListBuffer", vplListBuffer->GetShaderResourceView(0, 0, RENDER_FORMAT_UNKNOWN));

		int32_t index = 0;
		for (auto & volume : geometryVolumes)
		{
			cs->SetUAV("geometryVolumeRW" + std::to_string(index), volume->Get()->Cast<Texture>()->GetUnorderedAccessView(0, 0, 0));
			++index;
		}

		cs->Flush();

		int groupX = (geometryVolumes[0]->Get()->Cast<Texture>()->GetDesc().width + 3) / 4;
		int groupY = (geometryVolumes[0]->Get()->Cast<Texture>()->GetDesc().height + 3) / 4;
		int groupZ = (geometryVolumes[0]->Get()->Cast<Texture>()->GetDesc().depth + 3) / 4;

		Global::GetRenderEngine()->GetRenderContext()->Compute(groupX, groupY, groupZ);

		Global::GetRenderEngine()->GetRenderContext()->ResetShader(SHADER_CS);
	}

	void LPV::Inject(
		const Ptr<LightComponent> & light,
		const Ptr<RenderBuffer> & vplHeadBuffer,
		const Ptr<RenderBuffer> & vplListBuffer,
		const std::array<PooledTextureRef, 7> & lightVolumes)
	{
		std::map<String, String> macros;
		light->BindMacros(false, nullptr, macros);

		auto cs = Shader::FindOrCreate<LPVInjectCS>();

		light->BindShaderParams(cs, false, nullptr);

		cs->SetScalar("lpvGridDims", lpvGridDims);

		cs->SetSRV("vplHeadBuffer", vplHeadBuffer->GetShaderResourceView(0, 0, RENDER_FORMAT_R32_TYPELESS));
		cs->SetSRV("vplListBuffer", vplListBuffer->GetShaderResourceView(0, 0, RENDER_FORMAT_UNKNOWN));

		int32_t index = 0;
		for (auto & volume : lightVolumes)
		{
			cs->SetUAV("lightVolumeRW" + std::to_string(index), volume->Get()->Cast<Texture>()->GetUnorderedAccessView(0, 0, 0));
			++index;
		}

		cs->Flush();

		int groupX = (lightVolumes[0]->Get()->Cast<Texture>()->GetDesc().width + 3) / 4;
		int groupY = (lightVolumes[0]->Get()->Cast<Texture>()->GetDesc().height + 3) / 4;
		int groupZ = (lightVolumes[0]->Get()->Cast<Texture>()->GetDesc().depth + 3) / 4;

		Global::GetRenderEngine()->GetRenderContext()->Compute(groupX, groupY, groupZ);

		Global::GetRenderEngine()->GetRenderContext()->ResetShader(SHADER_CS);
	}

	void LPV::Propagate(
		const std::array<PooledTextureRef, 3> & geometryVolumes,
		std::array<PooledTextureRef, 7> & lightVolumes)
	{
		std::array<PooledTextureRef, 7> lightVolumesOrder;
		for (auto & volume : lightVolumesOrder)
		{
			volume = TexturePool::Instance().FindFree({ TEXTURE_3D, lightVolumes[0]->Get()->Cast<Texture>()->GetDesc() });
		}

		std::map<String, String> macros;
		if (_bGeometryOcclusion)
			macros["GEOMETRY_OCCLUSION"] = "";

		auto cs = Shader::FindOrCreate<LPVPropagateCS>(macros);

		cs->SetScalar("lpvGridDims", lpvGridDims);

		int groupX = (lightVolumes[0]->Get()->Cast<Texture>()->GetDesc().width + 3) / 4;
		int groupY = (lightVolumes[0]->Get()->Cast<Texture>()->GetDesc().height + 3) / 4;
		int groupZ = (lightVolumes[0]->Get()->Cast<Texture>()->GetDesc().depth + 3) / 4;

		for (int i = 0; i < _numPropagationItrs; ++i)
		{
			for (int i = 0; i < geometryVolumes.size(); ++i)
				cs->SetSRV("geometryVolume" + std::to_string(i), geometryVolumes[i]->Get()->Cast<Texture>()->GetShaderResourceView());

			for (int i = 0; i < lightVolumes.size(); ++i)
				cs->SetSRV("lightVolume" + std::to_string(i), lightVolumes[i]->Get()->Cast<Texture>()->GetShaderResourceView());

			for (int i = 0; i < lightVolumes.size(); ++i)
				cs->SetUAV("lightVolumeRW" + std::to_string(i), lightVolumesOrder[i]->Get()->Cast<Texture>()->GetUnorderedAccessView(0, 0, 0));

			cs->Flush();

			Global::GetRenderEngine()->GetRenderContext()->Compute(groupX, groupY, groupZ);

			Global::GetRenderEngine()->GetRenderContext()->ResetShader(SHADER_CS);

			for (int i = 0; i < lightVolumes.size(); ++i)
				lightVolumesOrder[i].swap(lightVolumes[i]);
		}

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);

		Global::GetRenderEngine()->GetRenderContext()->ResetShader(SHADER_CS);
	}

	void LPV::RenderIndirect(
		const Ptr<RenderView> & view,
		const float3 & worldToGridScale,
		const float3 & worldToGridOffset,
		const std::array<PooledTextureRef, 7> & lightVolumes,
		const Ptr<Texture> & gbuffer0,
		const Ptr<Texture> & gbuffer1,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<RenderTargetView> & target)
	{
		auto ps = Shader::FindOrCreate<RenderLPVIndirectPS>();

		view->BindShaderParams(ps);

		ps->SetScalar("lpvStrength", _lpvStrength);
		ps->SetScalar("worldToGridScale", worldToGridScale);
		ps->SetScalar("worldToGridOffset", worldToGridOffset);
		ps->SetScalar("lpvGridDims", lpvGridDims);

		for (int i = 0; i < lightVolumes.size(); ++i)
			ps->SetSRV("lightVolume" + std::to_string(i), lightVolumes[i]->Get()->Cast<Texture>()->GetShaderResourceView());

		ps->SetSRV("linearDepthTex", linearDepthTex->GetShaderResourceView());
		ps->SetSRV("gbuffer0", gbuffer0->GetShaderResourceView());
		ps->SetSRV("gbuffer1", gbuffer1->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE>::Get());

		DrawQuad({ target });

		rc->SetBlendState(nullptr);
	}
}