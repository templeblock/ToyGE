#pragma once
#ifndef LPV_H
#define LPV_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Texture;
	class LightComponent;

	struct ReflectiveShadowMap
	{
		Ptr<Texture> rsmFlux;
		Ptr<Texture> rsmDepth;
		Ptr<Texture> rsmNormal;
	};

	class TOYGE_CORE_API LPV : public RenderAction
	{
	public:
		LPV();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_SET(LPVStrength, float, _lpvStrength);
		CLASS_GET(LPVStrength, float, _lpvStrength);

		CLASS_SET(NumPropagationItrs, int32_t, _numPropagationItrs);
		CLASS_GET(NumPropagationItrs, int32_t, _numPropagationItrs);

		CLASS_SET(GeometryOcclusion, bool, _bGeometryOcclusion);
		CLASS_GET(GeometryOcclusion, bool, _bGeometryOcclusion);

	private:
		Ptr<RenderEffect> _fx;
		Ptr<RenderEffect> _renderFx;
		Ptr<RenderBuffer> _vplListBuffer;
		Ptr<RenderBuffer> _vplHeadBuffer;
		std::array<Ptr<Texture>, 7> _lightVolumes;
		std::array<Ptr<Texture>, 7> _lightVolumesBack;
		std::array<Ptr<Texture>, 3> _geometryVolumes;
		float3 _worldToGridScale;
		float3 _worldToGridOffset;
		float _lpvStrength;
		int32_t _numPropagationItrs;
		bool _bGeometryOcclusion;

		void ClearVPL();

		void GenerateVPLList(const Ptr<LightComponent> & light);

		void BuildGeometryVolume();

		void Inject();

		void Propagate();

		void RenderIndirect(
			const Ptr<Texture> & linearDepthTex,
			const Ptr<Texture> & gbuffer0,
			const Ptr<Texture> & gbuffer1,
			const Ptr<Texture> & gbuffer2,
			const Ptr<Texture> & targetTex);
	};
}

#endif
