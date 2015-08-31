#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\Math\Vector.h"

namespace ToyGE
{
	Camera::Camera(CameraType type)
		: _type(type),
		_pos(0.0f, 0.0f, 0.0f),
		_look(0.0f, 0.0f, 1.0f),
		_up(0.0f, 1.0f, 0.0f),
		_right(1.0f, 0.0f, 0.0f)
	{
		UpdateViewMatrix();
	}

	void Camera::SetPos(const XMFLOAT3 & pos)
	{
		_pos = pos;
		UpdateViewMatrix();
	}

	void Camera::SetViewMatrix(const XMFLOAT4X4 & matrix)
	{
		float3 pos = float3(-matrix._41, -matrix._42, -matrix._43);
		float3 x = float3(matrix._11, matrix._21, matrix._31);
		float3 y = float3(matrix._12, matrix._22, matrix._32);
		float3 z = float3(matrix._13, matrix._23, matrix._33);
		
		_pos = *reinterpret_cast<XMFLOAT3*>(&pos);
		_right = *reinterpret_cast<XMFLOAT3*>(&x);
		_up = *reinterpret_cast<XMFLOAT3*>(&y);
		_look = *reinterpret_cast<XMFLOAT3*>(&z);
		_viewMatrix = matrix;
	}

	void Camera::LookTo(const XMFLOAT3 & pos, const XMFLOAT3 & look, const XMFLOAT3 & up)
	{
		_pos = pos;
		_look = look;
		_up = up;
		UpdateViewMatrix();

		float3 look_ = *reinterpret_cast<const float3*>(&look);
		float3 up_ = *reinterpret_cast<const float3*>(&up);
		float3 right_ = cross(up_, look_);
		up_ = cross(look_, right_);

		_up = *reinterpret_cast<const XMFLOAT3*>(&up_);
		_right = *reinterpret_cast<const XMFLOAT3*>(&right_);
	}

	void Camera::LookAt(const XMFLOAT3 & lookPos)
	{
		if (lookPos.x == _pos.x && lookPos.y == _pos.y && lookPos.z == _pos.z)
			return;

		float3 lookPos_ = *reinterpret_cast<const float3*>(&lookPos);
		float3 pos_ = *reinterpret_cast<const float3*>(&_pos);
		float3 look = normalize(lookPos_ - pos_);

		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		if(std::abs(look.y) >= 0.99f)
			up = XMFLOAT3(1.0f, 0.0f, 0.0f);

		LookTo(_pos, *reinterpret_cast<const XMFLOAT3*>(&look), up);
	}

	void Camera::Walk(float value)
	{
		_pos.x += _look.x * value;
		_pos.y += _look.y * value;
		_pos.z += _look.z * value;
		UpdateViewMatrix();
	}

	void Camera::Strafe(float value)
	{
		_pos.x += _right.x * value;
		_pos.y += _right.y * value;
		_pos.z += _right.z * value;
		UpdateViewMatrix();
	}

	void Camera::Fly(float value)
	{
		_pos.x += _up.x * value;
		_pos.y += _up.y * value;
		_pos.z += _up.z * value;
		UpdateViewMatrix();
	}

	void Camera::Rotate(const XMFLOAT3 & axis, float angle)
	{
		auto xmAxis = XMLoadFloat3(&axis);
		auto rotate = XMMatrixRotationAxis(xmAxis, angle);
		auto xmRight = XMLoadFloat3(&_right);
		auto xmUp = XMLoadFloat3(&_up);
		auto xmLook = XMLoadFloat3(&_look);
		//auto xmPos = XMLoadFloat3(&_pos);
		xmRight = XMVector3TransformNormal(xmRight, rotate);
		xmUp = XMVector3TransformNormal(xmUp, rotate);
		xmLook = XMVector3TransformNormal(xmLook, rotate);
		//xmPos = XMVector3TransformCoord(xmPos, rotate);

		XMStoreFloat3(&_right, xmRight);
		XMStoreFloat3(&_up, xmUp);
		XMStoreFloat3(&_look, xmLook);
		//XMStoreFloat3(&_pos, xmPos);

		UpdateViewMatrix();
	}

	void Camera::Yaw(float angle)
	{
		Rotate(_up, angle);
	}

	void Camera::Pitch(float angle)
	{
		Rotate(_right, angle);
	}

	void Camera::Roll(float angle)
	{
		Rotate(_look, angle);
	}

	void Camera::UpdateViewMatrix()
	{
		auto xmEye = XMLoadFloat3(&_pos);
		auto xmFocus = XMVectorSet(_pos.x + _look.x, _pos.y + _look.y, _pos.z + _look.z, 1.0f);
		auto xmUp = XMLoadFloat3(&_up);
		auto xmView = XMMatrixLookAtLH(xmEye, xmFocus, xmUp);
		XMStoreFloat4x4(&_viewMatrix, xmView);
	}


	PerspectiveCamera::PerspectiveCamera()
		: Camera(CAMERA_PESPECTIVE)
	{

	}

	PerspectiveCamera::PerspectiveCamera(float fovAngle, float aspectRatio, float nearZ, float farZ)
		: Camera(CAMERA_PESPECTIVE),
		_fovAngle(fovAngle),
		_aspectRatio(aspectRatio)
	{
		_nearZ = nearZ;
		_farZ = farZ;
		
		UpdateProjMatrix();
	}

	void PerspectiveCamera::SetNearPlane(float nearPlane)
	{
		_nearZ = nearPlane;

		UpdateProjMatrix();
	}

	void PerspectiveCamera::SetFarPlane(float farPlane)
	{
		_farZ = farPlane;

		UpdateProjMatrix();
	}

	XNA::Frustum PerspectiveCamera::GetFrustum() const
	{
		using namespace XNA;
		XMMATRIX xmView = XMLoadFloat4x4(&ViewMatrix());
		xmView = XMMatrixInverse(&XMMatrixDeterminant(xmView), xmView);
		XMVECTOR scale;
		XMVECTOR rot;
		XMVECTOR trans;
		XMMatrixDecompose(&scale, &rot, &trans, xmView);
		XNA::Frustum ret = _frustum;
		ret.Origin = _pos;
		XMStoreFloat4(&ret.Orientation, rot);

		return ret;
	}

	void PerspectiveCamera::UpdateProjMatrix()
	{
		auto xmProj = XMMatrixPerspectiveFovLH(FovAngle(), AspectRatio(), _nearZ, _farZ);
		XMStoreFloat4x4(&_projMatrix, xmProj);

		using namespace XNA;
		//XMMATRIX xmProj = XMLoadFloat4x4(&ProjMatrix());
		ComputeFrustumFromProjection(&_frustum, &xmProj);
	}


	PhysicalCamera::PhysicalCamera(float nearZ, float farZ)
		: _fimSize(36.0f, 20.25f),
		_focalLength(12.0f),
		_focalDistance(1000.0f),
		_fstops(4.0f)
	{
		_nearZ = nearZ;
		_farZ = farZ;
		
		ReComputeProjAndFrustum();
	}

	void PhysicalCamera::ReComputeProjAndFrustum()
	{
		auto xmProj = XMMatrixPerspectiveFovLH(FovAngle(), AspectRatio(), _nearZ, _farZ);
		XMStoreFloat4x4(&_projMatrix, xmProj);

		using namespace XNA;
		//XMMATRIX xmProj = XMLoadFloat4x4(&ProjMatrix());
		ComputeFrustumFromProjection(&_frustum, &xmProj);
	}
}