#pragma once
#ifndef TRANSFORMCOMPONENT_H
#define TRANSFORMCOMPONENT_H

#include "ToyGE\RenderEngine\ActorComponent.h"
#include "ToyGE\Math\Math.h"

#include <boost\signals2.hpp>

namespace ToyGE
{
	class TOYGE_CORE_API TransformComponent : public ActorComponent
	{
	public:
		typedef void(TransformChangedEventType)(void);

		TransformComponent();

		virtual ~TransformComponent() = default;

		void AttachTo(const Ptr<TransformComponent> & parent);

		void SetPos(const float3 & pos)
		{
			_pos = pos;
			UpdateTransform();
		}
		const float3 & GetPos() const
		{
			return _pos;
		}

		void SetScale(const float3 & scale)
		{
			_scale = scale;
			UpdateTransform();
		}
		const float3 & GetScale() const
		{
			return _scale;
		}

		void SetOrientation(const Quaternion & orientation)
		{
			_orientation = orientation;
			UpdateTransform();
		}
		const Quaternion & GetOrientation() const
		{
			return _orientation;
		}

		void SetRelativeTransform(const float4x4 & transformMat);

		void SetWorldTransform(const float4x4 & transformMat);

		const float4x4 & GetRelativeTransformMatrix() const
		{
			return _relativetransformMatrix;
		}

		const float4x4 & GetWorldTransformMatrix() const
		{
			return _worldTransformMatrix;
		}

		void StoreWorldTransformCache()
		{
			_worldTransformMatrixCache = _worldTransformMatrix;
		}

		void BindShaderParams(const Ptr<class Shader> & shader);

		boost::signals2::signal<TransformChangedEventType> & OnTransformChanged()
		{
			return _transformChangedEvent;
		}

		void UpdateTransform();

	protected:
		Ptr<TransformComponent> _parent;
		float3 _pos;
		float3 _scale;
		Quaternion _orientation;
		float4x4 _worldTransformMatrix;
		float4x4 _relativetransformMatrix;
		float4x4 _worldTransformMatrixCache;

		boost::signals2::signal<TransformChangedEventType> _transformChangedEvent;

		
	};
}

#endif