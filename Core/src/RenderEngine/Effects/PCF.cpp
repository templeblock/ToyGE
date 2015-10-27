#include "ToyGE\RenderEngine\Effects\PCF.h"
#include "ToyGE\RenderEngine\RenderEffect.h"

namespace ToyGE
{
	PCF::PCF()
		: _filterSize(2.0f)
	{

	}

	Ptr<Texture> PCF::ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light)
	{
		return shadowMap;
	}

	void PCF::BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		fx->AddExtraMacro("SHADOW_TYPE", "SHADOW_TYPE_PCF");
	}

	void PCF::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		ShadowRenderTechnique::BindParams(fx, light, shadowMap);

		fx->VariableByName("pcfFilterSize")->AsScalar()->SetValue(&_filterSize);
		fx->VariableByName("shadowMapSize")->AsScalar()->SetValue(&shadowMap->GetTexSize());

		int32_t arraySize = shadowMap->Desc().arraySize;
		if (shadowMap->Desc().type == TEXTURE_CUBE)
			arraySize *= 6;
		fx->VariableByName("shadowTexArray")->AsShaderResource()->SetValue(shadowMap->CreateTextureView(0, 1, 0, arraySize));
	}
}