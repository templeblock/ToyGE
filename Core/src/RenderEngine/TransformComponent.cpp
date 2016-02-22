#include "ToyGE\RenderEngine\TransformComponent.h"
#include "ToyGE\RenderEngine\Shader.h"

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
		_orientation(identity_quat<float>()),
		_bDirty(false)
	{
		//XMStoreFloat4x4(&_transformMatrix, XMMatrixIdentity());
		//XMStoreFloat4x4(&_transformMatrixCache, XMMatrixIdentity());
		//XMStoreFloat4x4(&_relativetransformMatrix, XMMatrixIdentity());
		_transformMatrix = identity_mat<float, 4>();
		_transformMatrixCache = identity_mat<float, 4>();
		_relativetransformMatrix = identity_mat<float, 4>();
	}

	void TransformComponent::BindShaderParams(const Ptr<class Shader> & shader)
	{
		shader->SetScalar("localToWorldMatrix", _transformMatrix);
		shader->SetScalar("preLocalToWorldMatrix", _transformMatrixCache);
	}

	void TransformComponent::UpdateTransform()
	{
		//if (_bDirty)
		{
			UpdateTransformMatrix();
			OnTranformUpdated();
			_bDirty = false;
		}
	}

	void TransformComponent::AttachTo(const Ptr<TransformComponent> & parent)
	{
		_parent = parent;
		_bDirty = true;
		UpdateTransform();
	}

	void TransformComponent::UpdateTransformMatrix()
	{
		/*auto scaleXM = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
		auto translateXM = XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
		auto orientationXM = XMLoadFloat4(&_orientation);
		auto rotateXM = XMMatrixRotationQuaternion(orientationXM);
		auto relativeTransformXM = XMMatrixMultiply(XMMatrixMultiply(scaleXM, rotateXM), translateXM);
		XMStoreFloat4x4(&_relativetransformMatrix, relativeTransformXM);*/

		_relativetransformMatrix = transformation<float>(nullptr, nullptr, &_scale, nullptr, &_orientation, &_pos);

		auto parentTransform = _parent ? _parent->GetWorldTransformMatrix() : identity_mat<float, 4>();
		/*auto transformXM = XMMatrixMultiply(relativeTransformXM, parentTransform);
		XMStoreFloat4x4(&_transformMatrix, transformXM);*/
		_transformMatrix = mul(_relativetransformMatrix, parentTransform);
	}
}