//#include "ToyGE\RenderEngine\Effects\PCF.h"
//#include "ToyGE\RenderEngine\RenderEffect.h"
//
//namespace ToyGE
//{
//	PCF::PCF()
//		: _filterSize(0.05f)
//	{
//
//	}
//
//	Ptr<Texture> PCF::ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light)
//	{
//		return shadowMap;
//	}
//
//	void PCF::BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
//	{
//		fx->AddExtraMacro("SHADOW_TYPE", "SHADOW_TYPE_PCF");
//	}
//
//	void PCF::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
//	{
//		ShadowRenderTechnique::BindParams(fx, light, shadowMap);
//
//		fx->VariableByName("pcfFilterSize")->AsScalar()->SetValue(&_filterSize);
//	}
//}