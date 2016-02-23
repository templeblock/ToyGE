#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\Math\Vector.h"

namespace ToyGE
{
	Camera::Camera()
		: _pos(0.0f, 0.0f, 0.0f),
		_xAxis(1.0f, 0.0f, 0.0f),
		_yAxis(0.0f, 1.0f, 0.0f),
		_zAxis(0.0f, 0.0f, 1.0f)
	{
		_projMatrix = identity_mat<float, 4>();

		UpdateViewMatrix();
	}

	void Camera::SetViewMatrix(const float4x4 & matrix)
	{
		_pos = float3(-matrix(3, 0), -matrix(3, 1), -matrix(3, 2));
		_xAxis = float3(matrix(0, 0), matrix(1, 0), matrix(2, 0));
		_yAxis = float3(matrix(0, 1), matrix(1, 1), matrix(2, 1));
		_zAxis = float3(matrix(0, 2), matrix(1, 2), matrix(2, 2));
		_viewMatrix = matrix;

		_viewProjMatrix = mul(_viewMatrix, _projMatrix);
		UpdateFrustum();
	}

	void Camera::SetProjMatrix(const float4x4 & matrix)
	{
		_projMatrix = matrix;
		_viewProjMatrix = mul(_viewMatrix, _projMatrix);

		UpdateFrustum();
	}


	float4x4 Camera::GetViewProjMatrix() const
	{
		return _viewProjMatrix;
	}

	void Camera::SetPos(const float3 & pos)
	{
		_pos = pos;
		UpdateViewMatrix();
	}

	void Camera::LookTo(const float3 & pos, const float3 & look, const float3 & up)
	{
		_pos = pos;

		_zAxis = normalize( look );
		_yAxis = normalize( up );
		_xAxis = normalize( cross(_yAxis, _zAxis) );
		_yAxis = cross(_zAxis, _xAxis);

		UpdateViewMatrix();
	}

	void Camera::LookAt(const float3 & lookPos)
	{
		if (all(lookPos == _pos))
			return;

		float3 look = normalize(lookPos - _pos);

		float3 up = float3(0.0f, 1.0f, 0.0f);
		if(std::abs(look.y()) >= 0.99f)
			up = float3(1.0f, 0.0f, 0.0f);

		LookTo(_pos, look, up);
	}

	void Camera::Walk(float value)
	{
		_pos += _zAxis * value;
		UpdateViewMatrix();
	}

	void Camera::Strafe(float value)
	{
		_pos += _xAxis * value;
		UpdateViewMatrix();
	}

	void Camera::Fly(float value)
	{
		_pos += _yAxis * value;
		UpdateViewMatrix();
	}

	void Camera::Rotate(const float3 & axis, float angle)
	{
		auto rot = rotation_axis(axis, angle);
		_xAxis = transform_quat(_xAxis, rot);
		_yAxis = transform_quat(_yAxis, rot);
		_zAxis = transform_quat(_zAxis, rot);
		UpdateViewMatrix();
	}

	void Camera::Yaw(float angle)
	{
		Rotate(_yAxis, angle);
	}

	void Camera::Pitch(float angle)
	{
		Rotate(_xAxis, angle);
	}

	void Camera::Roll(float angle)
	{
		Rotate(_zAxis, angle);
	}

	void Camera::Cull(const Ptr<class SceneCuller> & culler, std::vector<Ptr<class Cullable>> & outElements)
	{
		culler->Cull(GetFrustum(), outElements);
	}

	void Camera::UpdateViewMatrix()
	{
		_viewMatrix = look_at_lh(_pos, _pos + _zAxis, _yAxis);
		_viewProjMatrix = mul(_viewMatrix, _projMatrix);

		UpdateFrustum();
	}

	void Camera::UpdateFrustum()
	{
		_frustum = compute_frustum_from_clip(_projMatrix, inverse(_projMatrix));
		_frustum = transform_frustum(_frustum, inverse(GetViewMatrix()));
	}

	PerspectiveCamera::PerspectiveCamera()
		: _fovAngle(PI_DIV2),
		_aspectRatio(1.0f)
	{
		_nearDepth = 1.0f;
		_farDepth = 100.0f;

		Camera::SetProjMatrix(perspective_fov_lh(GetFovAngle(), GetAspectRatio(), _nearDepth, _farDepth));
	}

	PerspectiveCamera::PerspectiveCamera(float fovAngle, float aspectRatio, float nearZ, float farZ)
		: _fovAngle(fovAngle),
		_aspectRatio(aspectRatio)
	{
		_nearDepth = nearZ;
		_farDepth = farZ;
		
		Camera::SetProjMatrix(perspective_fov_lh(GetFovAngle(), GetAspectRatio(), _nearDepth, _farDepth));
	}

	void PerspectiveCamera::SetProjMatrix(const float4x4 & matrix)
	{
		Camera::SetProjMatrix(matrix);

		_nearDepth = -matrix(3, 3) / matrix(2, 3);
		_farDepth = matrix(2, 3) * _nearDepth / (matrix(2, 3) - 1.0f);
		_fovAngle = std::atan(1.0f / matrix(1, 1)) * 2.0f;
		_aspectRatio = matrix(1, 1) / matrix(0, 0);
	}

	void PerspectiveCamera::SetFovAngle(float angle)
	{
		_fovAngle = angle;
		Camera::SetProjMatrix(perspective_fov_lh(GetFovAngle(), GetAspectRatio(), _nearDepth, _farDepth));
	}

	void PerspectiveCamera::SetAspectRatio(float ratio)
	{
		_aspectRatio = ratio;
		Camera::SetProjMatrix(perspective_fov_lh(GetFovAngle(), GetAspectRatio(), _nearDepth, _farDepth));
	}

	void PerspectiveCamera::SetNear(float nearDepth)
	{
		Camera::SetNear(nearDepth);
		Camera::SetProjMatrix(perspective_fov_lh(GetFovAngle(), GetAspectRatio(), _nearDepth, _farDepth));
	}

	void PerspectiveCamera::SetFar(float farDepth)
	{
		Camera::SetFar(farDepth);
		Camera::SetProjMatrix(perspective_fov_lh(GetFovAngle(), GetAspectRatio(), _nearDepth, _farDepth));
	}


	OrthogonalCamera::OrthogonalCamera()
	{
		UpdateProjMatrix();
	}

	OrthogonalCamera::OrthogonalCamera(float left, float right, float bottom, float up, float front, float back)
	{
		SetViewBox(left, right, bottom, up, front, back);
	}

	void OrthogonalCamera::SetProjMatrix(const float4x4 & matrix)
	{
		Camera::SetProjMatrix(matrix);

		float3 scale = float3(matrix(0, 0), matrix(1, 1), matrix(2, 2));
		float3 offset = float3(matrix(3, 0), matrix(3, 1), matrix(3, 2));
		float3 min, max;
		min.x() = (-1.0f - offset.x()) / scale.x();
		max.x() = (1.0f - offset.x()) / scale.x();
		min.y() = (-1.0f - offset.y()) / scale.y();
		max.y() = (1.0f - offset.y()) / scale.y();
		min.z() = -offset.z() / scale.z();
		max.z() = 1.0f / scale.x() + min.z();

		_left = min.x();
		_right = max.x();
		_bottom = min.y();
		_up = max.y();
		_nearDepth = min.z();
		_farDepth = max.z();
	}

	void OrthogonalCamera::SetNear(float nearDepth)
	{
		Camera::SetNear(nearDepth);
		UpdateProjMatrix();
	}

	void OrthogonalCamera::SetFar(float farDepth)
	{
		Camera::SetFar(farDepth);
		UpdateProjMatrix();
	}

	void OrthogonalCamera::SetViewBox(float left, float right, float bottom, float up, float front, float back)
	{
		_left = left;
		_right = right;
		_bottom = bottom;
		_up = up;
		_nearDepth = front;
		_farDepth = back;

		UpdateProjMatrix();
	}

	void OrthogonalCamera::UpdateProjMatrix()
	{
		float3 min = float3(_left, _bottom, _nearDepth);
		float3 max = float3(_right, _up, _farDepth);

		float3 scale;
		float3 offset;
		scale.x() = 2.0f / (max.x() - min.x());
		scale.y() = 2.0f / (max.y() - min.y());
		scale.z() = 1.0f / (max.z() - min.z());
		offset.x() = (max.x() + min.x()) * -0.5f * scale.x();
		offset.y() = (max.y() + min.y()) * -0.5f * scale.y();
		offset.z() = -min.z() * scale.z();

		auto projMatrix = float4x4(
		{
			scale.x(), 0.0f, 0.0f, 0.0f,
			0.0f, scale.y(), 0.0f, 0.0f,
			0.0f, 0.0f, scale.z(), 0.0f,
			offset.x(), offset.y(), offset.z(), 1.0f
		});

		Camera::SetProjMatrix(projMatrix);
	}

	

	//PhysicalCamera::PhysicalCamera(float nearZ, float farZ)
	//	: _fimSize(36.0f, 20.25f),
	//	_focalLength(12.0f),
	//	_focalDistance(1000.0f),
	//	_fstops(4.0f)
	//{
	//	_nearZ = nearZ;
	//	_farZ = farZ;
	//	
	//	ReComputeProjAndFrustum();
	//}

	//void PhysicalCamera::ReComputeProjAndFrustum()
	//{
	//	auto xmProj = XMMatrixPerspectiveFovLH(FovAngle(), AspectRatio(), _nearZ, _farZ);
	//	XMStoreFloat4x4(&_projMatrix, xmProj);

	//	using namespace XNA;
	//	//XMMATRIX xmProj = XMLoadFloat4x4(&ProjMatrix());
	//	ComputeFrustumFromProjection(&_frustum, &xmProj);
	//}
}