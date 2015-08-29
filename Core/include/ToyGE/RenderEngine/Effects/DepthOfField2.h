//#pragma once
//#ifndef DEPTHOFFIELD2_H
//#define DEPTHOFFIELD2_H
//
//#include "ToyGE\RenderEngine\RenderAction.h"
//
//namespace ToyGE
//{
//	class Camera;
//
//	class TOYGE_CORE_API DepthOfField2 : public RenderAction
//	{
//	public:
//		DepthOfField2();
//
//		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;
//
//	private:
//		Ptr<RenderEffect> _fx;
//
//		Ptr<Texture> ComputeCoC(const Ptr<Texture> & linearDepthTex, const Ptr<Texture> & rawDepth, const Ptr<Camera> & camera);
//
//		Ptr<Texture> ComputeTileMax(const Ptr<Texture> & cocTex, const Ptr<Texture> & linearDepthTex);
//
//		Ptr<Texture> ComputeAlpha(
//			const Ptr<Texture> & cocTex,
//			const Ptr<Texture> & tileMaxTex,
//			const Ptr<Texture> & linearDepthTex);
//
//		void Blur(
//			const Ptr<Texture> & sceneTex,
//			const Ptr<Texture> & tileMaxTex, 
//			const Ptr<Texture> & alphaTex,
//			const Ptr<Texture> & targetTex);
//	};
//
//}
//
//#endif