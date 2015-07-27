//#pragma once
//#ifndef ALCHEMYAO_H
//#define ALCHEMYAO_H
//
////#include "ToyGE\RenderEngine\RenderAction.h"
//#include "ToyGE\Kernel\PreIncludes.h"
//#include "ToyGE\Kernel\CorePreDeclare.h"
//
//namespace ToyGE
//{
//	class TOYGE_CORE_API AlchemyAO
//	{
//	public:
//		AlchemyAO();
//
//		Ptr<Texture> GetAOTexture(const Ptr<Texture> & depthLinearTex, const Ptr<Texture> & gbuffer0, const Ptr<Camera> & camera);
//
//		void SetSampleRadius(float radius)
//		{
//			_radius = radius;
//		}
//		float SampleRadius() const
//		{
//			return _radius;
//		}
//		void SetBias(float bias)
//		{
//			_bias = bias;
//		}
//		float Bias() const
//		{
//			return _bias;
//		}
//		void SetIntensity(float intensity)
//		{
//			_intensity = intensity;
//		}
//		float Intensity() const
//		{
//			return _intensity;
//		}
//
//
//	private:
//		Ptr<RenderEffect> _ssaoFX;
//		Ptr<Texture> _offsetMap;
//
//		float _radius;
//		float _bias;
//		float _intensity;
//
//		void InitOffsetMap();
//		Ptr<Texture> AlchemyAO(const Ptr<Texture> & gbuffer, const Ptr<Camera> & camera);
//		Ptr<Texture> ScalableAO(const Ptr<Texture> & depthLinearTex, const Ptr<Texture> & gbuffer0, const Ptr<Camera> & camera);
//	};
//}
//
//#endif