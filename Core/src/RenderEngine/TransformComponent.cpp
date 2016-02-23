#include "ToyGE\RenderEngine\TransformComponent.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	TransformComponent::TransformComponent()
		: _pos(0.0f, 0.0f, 0.0f),
		_scale(1.0f, 1.0f, 1.0f),
		_orientation(identity_quat<float>())
	{
		_relativetransformMatrix = identity_mat<float, 4>();
		_worldTransformMatrix = identity_mat<float, 4>();
		_worldTransformMatrixCache = identity_mat<float, 4>();
	}

	void TransformComponent::AttachTo(const Ptr<TransformComponent> & parent)
	{
		_parent = parent;
		UpdateTransform();
	}

	void TransformComponent::SetRelativeTransform(const float4x4 & transformMat)
	{
		_relativetransformMatrix = transformMat;
		decompose(transformMat, _scale, _orientation, _pos);

		auto parentTransform = _parent ? _parent->GetWorldTransformMatrix() : identity_mat<float, 4>();
		_worldTransformMatrix = mul(_relativetransformMatrix, parentTransform);

		_transformChangedEvent();
	}

	void TransformComponent::SetWorldTransform(const float4x4 & transformMat)
	{
		_worldTransformMatrix = transformMat;

		auto invParentTransform = _parent ? inverse(_parent->GetWorldTransformMatrix()) : identity_mat<float, 4>();
		_relativetransformMatrix = mul(invParentTransform, transformMat);
		decompose(transformMat, _scale, _orientation, _pos);

		_transformChangedEvent();
	}

	void TransformComponent::BindShaderParams(const Ptr<class Shader> & shader)
	{
		shader->SetScalar("localToWorldMatrix", _worldTransformMatrix);
		shader->SetScalar("preLocalToWorldMatrix", _worldTransformMatrixCache);
	}

	void TransformComponent::UpdateTransform()
	{
		_relativetransformMatrix = transformation<float>(nullptr, nullptr, &_scale, nullptr, &_orientation, &_pos);

		auto parentTransform = _parent ? _parent->GetWorldTransformMatrix() : identity_mat<float, 4>();
		_worldTransformMatrix = mul(_relativetransformMatrix, parentTransform);

		_transformChangedEvent();
	}
}