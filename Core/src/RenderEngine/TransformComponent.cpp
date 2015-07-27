#include "ToyGE\RenderEngine\TransformComponent.h"

namespace ToyGE
{
	String TransformComponent::Name()
	{
		return "TRANSFORMCOMPONENT";
	}

	String TransformComponent::GetComponentName() const
	{
		return TransformComponent::Name();
	}

	TransformComponent::TransformComponent()
		: _pos(0.0f, 0.0f, 0.0f),
		_scale(1.0f, 1.0f, 1.0f),
		_orientation(0.0f, 0.0f, 0.0f, 1.0f),
		_bDirty(false)
	{
		XMStoreFloat4x4(&_transformMatrix, XMMatrixIdentity());
	}

	void TransformComponent::UpdateTransform()
	{
		if (_bDirty)
		{
			UpdateTransformMatrix();
			OnTranformUpdated();
			_bDirty = false;
		}
	}

	void TransformComponent::UpdateTransformMatrix()
	{
		auto scaleXM = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
		auto translateXM = XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
		auto orientationXM = XMLoadFloat4(&_orientation);
		auto rotateXM = XMMatrixRotationQuaternion(orientationXM);
		auto transformXM = XMMatrixMultiply(XMMatrixMultiply(scaleXM, rotateXM), translateXM);
		XMStoreFloat4x4(&_transformMatrix, transformXM);
	}
}