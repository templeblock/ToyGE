#pragma once
#ifndef DEPTHOFFIELD_H
#define DEPTHOFFIELD_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class Texture;
	class Camera;

	class TOYGE_CORE_API DepthOfField : public RenderAction
	{
	public:
		DepthOfField();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _dofFX;

		Ptr<Texture> ComputeCoC(const Ptr<Texture> & sceneTex, const Ptr<Texture> & linearDepthTex, const Ptr<Camera> & camera);

		std::pair<Ptr<Texture>, Ptr<Texture>> Blur(const Ptr<Texture> & packTex);

		void Composite(const Ptr<Texture> & packTex, const Ptr<Texture> & nearFieldTex, const Ptr<Texture> & blurTex, const ResourceView & target);
	};
}

#endif