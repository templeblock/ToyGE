#pragma once
#ifndef AMBIENTMAP_H
#define AMBIENTMAP_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	enum AmbientMapType
	{
		AM_CUBEMAP,
		AM_PANORAMIC,
		AM_HEMIPANORAMIC
	};

	class TOYGE_CORE_API AmbientMap
	{
	public:
		static Ptr<AmbientMap> Create(AmbientMapType type);

		AmbientMap(AmbientMapType type)
			: _type(type)
		{

		}

		virtual ~AmbientMap() = default;

		CLASS_GET(Type, AmbientMapType, _type);

		virtual void Render(const Ptr<class RenderView> & view);

		CLASS_SET(Texture, Ptr<Texture>, _tex)
		CLASS_GET(Texture, Ptr<Texture>, _tex)

	protected:
		Ptr<class Mesh> _sphereMesh;
		Ptr<Texture> _tex;

	private:
		AmbientMapType _type;
		//void Bind(const Ptr<class RenderView> & view, Ptr<Shader> & vs, Ptr<Shader> & ps);
	};
}

#endif