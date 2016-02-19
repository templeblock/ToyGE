#pragma once
#ifndef LPV_H
#define LPV_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	DECLARE_SHADER(, ClearVPLsCS, SHADER_CS, "LPV", "ClearVPLsCS", SM_5);
	DECLARE_SHADER(, GenerateVPLsCS, SHADER_CS, "LPV", "GenerateVPLsCS", SM_5);
	DECLARE_SHADER(, GVInjectCS, SHADER_CS, "LPV", "GVInjectCS", SM_5);
	DECLARE_SHADER(, LPVInjectCS, SHADER_CS, "LPV", "LPVInjectCS", SM_5);
	DECLARE_SHADER(, LPVPropagateCS, SHADER_CS, "LPV", "LPVPropagateCS", SM_5);

	DECLARE_SHADER(, LPVAccumPS, SHADER_PS, "LPVAccum", "LPVAccumPS", SM_4);

	DECLARE_SHADER(, RenderLPVIndirectPS, SHADER_PS, "LPVRenderIndirect", "RenderLPVIndirectPS", SM_4);

	class Texture;
	class LightComponent;

	class TOYGE_CORE_API LPV : public RenderAction
	{
	public:
		LPV();

		void Render(const Ptr<RenderView> & view) override;

		CLASS_SET(LPVStrength, float, _lpvStrength);
		CLASS_GET(LPVStrength, float, _lpvStrength);

		CLASS_SET(NumPropagationItrs, int32_t, _numPropagationItrs);
		CLASS_GET(NumPropagationItrs, int32_t, _numPropagationItrs);

		CLASS_SET(GeometryOcclusion, bool, _bGeometryOcclusion);
		CLASS_GET(GeometryOcclusion, bool, _bGeometryOcclusion);

	private:
		float _lpvStrength;
		int32_t _numPropagationItrs;
		bool _bGeometryOcclusion;

		void ClearVPL(const Ptr<RenderBuffer> & vplHeadBuffer);

		void GenerateVPLList(
			const Ptr<LightComponent> & light,
			const float3 & worldToGridScale,
			const float3 & worldToGridOffset,
			const Ptr<RenderBuffer> & vplHeadBuffer,
			const Ptr<RenderBuffer> & vplListBuffer);

		void BuildGeometryVolume(
			const Ptr<LightComponent> & light,
			const Ptr<RenderBuffer> & vplHeadBuffer,
			const Ptr<RenderBuffer> & vplListBuffer,
			const std::array<PooledTextureRef, 3> & geometryVolumes);

		void Inject(
			const Ptr<LightComponent> & light,
			const Ptr<RenderBuffer> & vplHeadBuffer,
			const Ptr<RenderBuffer> & vplListBuffer,
			const std::array<PooledTextureRef, 7> & lightVolumes);

		void Propagate(
			const std::array<PooledTextureRef, 3> & geometryVolumes,
			std::array<PooledTextureRef, 7> & lightVolumes);

		void RenderIndirect(
			const Ptr<RenderView> & view,
			const float3 & worldToGridScale,
			const float3 & worldToGridOffset,
			const std::array<PooledTextureRef, 7> & lightVolumes,
			const Ptr<Texture> & gbuffer0,
			const Ptr<Texture> & gbuffer1,
			const Ptr<Texture> & linearDepthTex,
			const Ptr<RenderTargetView> & target);
	};
}

#endif
