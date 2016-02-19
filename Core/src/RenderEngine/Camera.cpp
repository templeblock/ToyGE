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
		_xAxis = *reinterpret_cast<XMFLOAT3*>(&x);
		_yAxis = *reinterpret_cast<XMFLOAT3*>(&y);
		_zAxis = *reinterpret_cast<XMFLOAT3*>(&z);
		_viewMatrix = matrix;
	}

	XMFLOAT4X4 Camera::GetViewProjMatrix() const
	{
		auto viewXM = XMLoadFloat4x4(&GetViewMatrix());
		auto projXM = XMLoadFloat4x4(&GetProjMatrix());
		auto viewProjXM = XMMatrixMultiply(viewXM, projXM);
		XMFLOAT4X4 viewProj;
		XMStoreFloat4x4(&viewProj, viewProjXM);
		return viewProj;
	}

	void Camera::LookTo(const XMFLOAT3 & pos, const XMFLOAT3 & look, const XMFLOAT3 & up)
	{
		_pos = pos;

		float3 zAxis = normalize( *reinterpret_cast<const float3*>(&look) );
		float3 yAxis = normalize( *reinterpret_cast<const float3*>(&up) );
		float3 xAxis = normalize( cross(yAxis, zAxis) );
		yAxis = cross(zAxis, xAxis);

		_xAxis = *reinterpret_cast<const XMFLOAT3*>(&xAxis);
		_yAxis = *reinterpret_cast<const XMFLOAT3*>(&yAxis);
		_zAxis = *reinterpret_cast<const XMFLOAT3*>(&zAxis);

		UpdateViewMatrix();
	}

	void Camera::LookAt(const XMFLOAT3 & lookPos)
	{
		if (lookPos.x == _pos.x && lookPos.y == _pos.y && lookPos.z == _pos.z)
			return;

		float3 lookPos_ = *reinterpret_cast<const float3*>(&lookPos);
		float3 pos_ = *reinterpret_cast<const float3*>(&_pos);
		float3 look = normalize(lookPos_ - pos_);

		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		if(std::abs(look.y()) >= 0.99f)
			up = XMFLOAT3(1.0f, 0.0f, 0.0f);

		LookTo(_pos, *reinterpret_cast<const XMFLOAT3*>(&look), up);
	}

	void Camera::Walk(float value)
	{
		_pos.x += _zAxis.x * value;
		_pos.y += _zAxis.y * value;
		_pos.z += _zAxis.z * value;
		UpdateViewMatrix();
	}

	void Camera::Strafe(float value)
	{
		_pos.x += _xAxis.x * value;
		_pos.y += _xAxis.y * value;
		_pos.z += _xAxis.z * value;
		UpdateViewMatrix();
	}

	void Camera::Fly(float value)
	{
		_pos.x += _yAxis.x * value;
		_pos.y += _yAxis.y * value;
		_pos.z += _yAxis.z * value;
		UpdateViewMatrix();
	}

	void Camera::Rotate(const XMFLOAT3 & axis, float angle)
	{
		auto xmAxis = XMLoadFloat3(&axis);
		auto rotate = XMMatrixRotationAxis(xmAxis, angle);
		auto xmRight = XMLoadFloat3(&_xAxis);
		auto xmUp = XMLoadFloat3(&_yAxis);
		auto xmLook = XMLoadFloat3(&_zAxis);
		//auto xmPos = XMLoadFloat3(&_pos);
		xmRight = XMVector3TransformNormal(xmRight, rotate);
		xmUp = XMVector3TransformNormal(xmUp, rotate);
		xmLook = XMVector3TransformNormal(xmLook, rotate);
		//xmPos = XMVector3TransformCoord(xmPos, rotate);

		XMStoreFloat3(&_xAxis, xmRight);
		XMStoreFloat3(&_yAxis, xmUp);
		XMStoreFloat3(&_zAxis, xmLook);
		//XMStoreFloat3(&_pos, xmPos);

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

	void Camera::UpdateViewMatrix()
	{
		auto xmEye = XMLoadFloat3(&_pos);
		auto xmFocus = XMVectorSet(_pos.x + _zAxis.x, _pos.y + _zAxis.y, _pos.z + _zAxis.z, 1.0f);
		auto xmUp = XMLoadFloat3(&_yAxis);
		auto xmView = XMMatrixLookAtLH(xmEye, xmFocus, xmUp);
		XMStoreFloat4x4(&_viewMatrix, xmView);
	}


	PerspectiveCamera::PerspectiveCamera(float fovAngle, float aspectRatio, float nearZ, float farZ)
		: _fovAngle(fovAngle),
		_aspectRatio(aspectRatio)
	{
		_nearDepth = nearZ;
		_farDepth = farZ;
		
		UpdateProjMatrix();
	}

	void PerspectiveCamera::SetProjMatrix(const XMFLOAT4X4 & matrix)
	{
		Camera::SetProjMatrix(matrix);

		_nearDepth = -matrix._44 / matrix._34;
		_farDepth = matrix._34 * _nearDepth / (matrix._34 - 1.0f);
		_fovAngle = std::atan(1.0f / matrix._22) * 2.0f;
		_aspectRatio = matrix._22 / matrix._11;

		using namespace XNA;
		XMMATRIX projXM = XMLoadFloat4x4(&GetProjMatrix());
		ComputeFrustumFromProjection(&_frustum, &projXM);
	}

	void PerspectiveCamera::SetFovAngle(float angle)
	{
		_fovAngle = angle;
		UpdateProjMatrix();
	}

	void PerspectiveCamera::SetAspectRatio(float ratio)
	{
		_aspectRatio = ratio;
		UpdateProjMatrix();
	}

	void PerspectiveCamera::SetNear(float nearDepth)
	{
		Camera::SetNear(nearDepth);
		UpdateProjMatrix();
	}

	void PerspectiveCamera::SetFar(float farDepth)
	{
		Camera::SetFar(farDepth);
		UpdateProjMatrix();
	}

	XNA::Frustum PerspectiveCamera::GetFrustum() const
	{
		using namespace XNA;
		XMMATRIX xmView = XMLoadFloat4x4(&GetViewMatrix());
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

	void PerspectiveCamera::Cull(const Ptr<class SceneCuller> & culler, std::vector<Ptr<class Cullable>> & outElements)
	{
		culler->Cull(GetFrustum(), outElements);
	}

	void PerspectiveCamera::UpdateProjMatrix()
	{
		auto xmProj = XMMatrixPerspectiveFovLH(GetFovAngle(), GetAspectRatio(), _nearDepth, _farDepth);
		XMStoreFloat4x4(&_projMatrix, xmProj);

		using namespace XNA;
		//XMMATRIX xmProj = XMLoadFloat4x4(&ProjMatrix());
		ComputeFrustumFromProjection(&_frustum, &xmProj);
	}


	OrthogonalCamera::OrthogonalCamera()
	{
		UpdateProjMatrix();
	}

	void OrthogonalCamera::SetProjMatrix(const XMFLOAT4X4 & matrix)
	{
		Camera::SetProjMatrix(matrix);
		float3 scale = float3(matrix._11, matrix._22, matrix._33);
		float3 offset = float3(matrix._41, matrix._42, matrix._43);
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

	void OrthogonalCamera::Cull(const Ptr<class SceneCuller> & culler, std::vector<Ptr<class Cullable>> & outElements)
	{
		float3 viewMin = float3(_left, _bottom, _nearDepth);
		float3 viewMax = float3(_right, _up, _farDepth);

		XNA::AxisAlignedBox viewAABB;
		Math::MinMaxToAxisAlignedBox(viewMin, viewMax, viewAABB);
		auto viewXM = XMLoadFloat4x4(&GetViewMatrix());
		auto invViewXM = XMMatrixInverse(&XMMatrixDeterminant(viewXM), viewXM);
		float3 center;
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&center), XMVector3TransformCoord(XMLoadFloat3(&viewAABB.Center), invViewXM));

		float3 min = FLT_MAX;
		float3 max = -FLT_MAX;

		float3 xAxis = *reinterpret_cast<const float3*>(&_xAxis);
		float3 yAxis = *reinterpret_cast<const float3*>(&_yAxis);
		float3 zAxis = *reinterpret_cast<const float3*>(&_zAxis);

		for (int i = 0; i < 8; ++i)
		{
			float3 p = center;
			p += (i & 1) ? xAxis * viewAABB.Extents.x : xAxis * -viewAABB.Extents.x;
			p += (i & 2) ? yAxis * viewAABB.Extents.y : yAxis * -viewAABB.Extents.y;
			p += (i & 4) ? zAxis * viewAABB.Extents.z : zAxis * -viewAABB.Extents.z;
			min = min_vec(p, min);
			max = max_vec(p, max);
		}
		XNA::AxisAlignedBox aabb;
		Math::MinMaxToAxisAlignedBox(min, max, aabb);

		culler->Cull(aabb, outElements);
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

		auto projMatXM = XMMatrixSet
			(
				scale.x(), 0.0f, 0.0f, 0.0f,
				0.0f, scale.y(), 0.0f, 0.0f,
				0.0f, 0.0f, scale.z(), 0.0f,
				offset.x(), offset.y(), offset.z(), 1.0f
				);
		XMStoreFloat4x4(&_projMatrix, projMatXM);
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