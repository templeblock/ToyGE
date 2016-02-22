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

		void SetPos(const float3 & pos)
		{
			_pos = pos;
			_bDirty = true;
		}

		virtual const float3 & GetPos() const
		{
			return _pos;
		}

		void SetScale(const float3 & scale)
		{
			_scale = scale;
			_bDirty = true;
		}

		const float3 & GetScale() const
		{
			return _scale;
		}

		void SetOrientation(const Quaternion & orientation)
		{
			_orientation = orientation;
			_bDirty = true;
		}

		const Quaternion & GetOrientation() const
		{
			return _orientation;
		}

		const float4x4 & GetTransformMatrix() const
		{
			return _relativetransformMatrix;
		}

		const float4x4 & GetWorldTransformMatrix() const
		{
			return _transformMatrix;
		}

		void UpdateTransform();

		CLASS_GET(TransformMatrixCache, float4x4, _transformMatrixCache);
		CLASS_SET(TransformMatrixCache, float4x4, _transformMatrixCache);

		void BindShaderParams(const Ptr<class Shader> & shader);

		void AttachTo(const Ptr<TransformComponent> & parent);

	protected:
		Ptr<TransformComponent> _parent;
		float3 _pos;
		float3 _scale;
		Quaternion _orientation;
		float4x4 _transformMatrix;
		float4x4 _relativetransformMatrix;
		bool _bDirty;
		float4x4 _transformMatrixCache;

		virtual void OnTranformUpdated(){};

	private:
		void UpdateTransformMatrix();
	};
}

#endif