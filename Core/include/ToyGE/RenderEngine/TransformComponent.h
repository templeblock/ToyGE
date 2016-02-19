#pragma once
#ifndef TRANSFORMCOMPONENT_H
#define TRANSFORMCOMPONENT_H

#include "ToyGE\RenderEngine\SceneObjectComponent.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class TOYGE_CORE_API TransformComponent : public SceneObjectComponent
	{
	public:
		static String Name();

		TransformComponent();

		String GetComponentName() const override;

		void SetPos(const XMFLOAT3 & pos)
		{
			_pos = pos;
			_bDirty = true;
		}

		virtual const XMFLOAT3 & GetPos() const
		{
			return _pos;
		}

		void SetScale(const XMFLOAT3 & scale)
		{
			_scale = scale;
			_bDirty = true;
		}

		const XMFLOAT3 & GetScale() const
		{
			return _scale;
		}

		void SetOrientation(const XMFLOAT4 & orientation)
		{
			_orientation = orientation;
			_bDirty = true;
		}

		const XMFLOAT4 & GetOrientation() const
		{
			return _orientation;
		}

		const XMFLOAT4X4 & GetTransformMatrix() const
		{
			return _relativetransformMatrix;
		}

		const XMFLOAT4X4 & GetWorldTransformMatrix() const
		{
			return _transformMatrix;
		}

		void UpdateTransform();

		CLASS_GET(TransformMatrixCache, XMFLOAT4X4, _transformMatrixCache);
		CLASS_SET(TransformMatrixCache, XMFLOAT4X4, _transformMatrixCache);

		void BindShaderParams(const Ptr<class Shader> & shader);

		void AttachTo(const Ptr<TransformComponent> & parent);

	protected:
		Ptr<TransformComponent> _parent;
		XMFLOAT3 _pos;
		XMFLOAT3 _scale;
		XMFLOAT4 _orientation;
		XMFLOAT4X4 _transformMatrix;
		XMFLOAT4X4 _relativetransformMatrix;
		bool _bDirty;
		XMFLOAT4X4 _transformMatrixCache;

		virtual void OnTranformUpdated(){};

	private:
		void UpdateTransformMatrix();
	};
}

#endif