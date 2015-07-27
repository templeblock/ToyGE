#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"


namespace ToyGE
{
	enum CameraType
	{
		CAMERA_PESPECTIVE
	};

	class TOYGE_CORE_API Camera
	{
	public:
		Camera(CameraType type);

		virtual ~Camera() = default;

		CameraType Type() const
		{
			return _type;
		}

		void SetPos(const XMFLOAT3 & pos);

		const XMFLOAT3 & Pos() const
		{
			return _pos;
		}

		void SetViewMatrix(const XMFLOAT4X4 & matrix);

		void LookTo(const XMFLOAT3 & pos, const XMFLOAT3 & look, const XMFLOAT3 & up);

		const XMFLOAT4X4 & ViewMatrix() const
		{
			return _viewMatrix;
		}

		const XMFLOAT3 & Look() const
		{
			return _look;
		}

		const XMFLOAT3 & Up() const
		{
			return _up;
		}

		const XMFLOAT3 & Right() const
		{
			return _right;
		}

		float Near() const
		{
			return _nearZ;
		}

		float Far() const
		{
			return _farZ;
		}

		virtual const XMFLOAT4X4 & ProjMatrix() const = 0;
		virtual XNA::Frustum GetFrustum() const = 0;

		void Walk(float value);
		void Strafe(float value);
		void Fly(float value);

		void Rotate(const XMFLOAT3 & axis, float angle);
		void Yaw(float angle);
		void Pitch(float angle);
		void Roll(float angle);

		CLASS_GET(ViewMatrixCache, XMFLOAT4X4, _viewMatrixCache);
		CLASS_SET(ViewMatrixCache, XMFLOAT4X4, _viewMatrixCache);

	protected:
		CameraType _type;
		XMFLOAT3 _pos;
		XMFLOAT3 _right;
		XMFLOAT3 _up;
		XMFLOAT3 _look;
		XMFLOAT4X4 _viewMatrix;
		XMFLOAT4X4 _viewMatrixCache;
		float _nearZ;
		float _farZ;

		void UpdateViewMatrix();
	};

	class TOYGE_CORE_API PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera();

		PerspectiveCamera(float fovAngle, float aspectRatio, float nearZ, float farZ);

		const XMFLOAT4X4 & ProjMatrix() const override
		{
			return _projMatrix;
		}

		XNA::Frustum GetFrustum() const override;

		virtual float FovAngle() const
		{
			return _fovAngle;
		}

		virtual float AspectRatio() const
		{
			return _aspectRatio;
		}

	protected:
		XMFLOAT4X4 _projMatrix;
		XNA::Frustum _frustum;
		float _fovAngle;
		float _aspectRatio;
	};

	class TOYGE_CORE_API PhysicalCamera : public PerspectiveCamera
	{
	public:
		PhysicalCamera(float nearZ, float farZ);

		float FovAngle() const override
		{
			float v = _focalDistance * _focalLength / (_focalDistance - _focalLength);
			return std::atan(_fimSize.y / (2.0f * v)) * 2.0f;
		}

		float AspectRatio() const override
		{
			return _fimSize.x / _fimSize.y;
		}

		void SetFilmSize(const float2 & filmSize)
		{
			_fimSize = filmSize;
			ReComputeProjAndFrustum();
		}

		const float2 & GetFilmSize() const
		{
			return _fimSize;
		}

		void SetFocalLength(float focalLength)
		{
			_focalLength = focalLength;
			ReComputeProjAndFrustum();
		}

		const float & GetFocalLength() const
		{
			return _focalLength;
		}

		void SetFocalDistance(float focalDistance)
		{
			_focalDistance = focalDistance;
			ReComputeProjAndFrustum();
		}

		const float & GetFocalDistance() const
		{
			return _focalDistance;
		}

		void SetFStops(float fstops)
		{
			_fstops = fstops;
		}

		const float & GetFStops() const
		{
			return _fstops;
		}

	private:
		//XMFLOAT4X4 _projMatrix;
		//XNA::Frustum _frustum;
		//float _fovAngle;
		//float _aspectRatio;
		float2 _fimSize;
		float _focalLength;
		float _focalDistance;
		float _fstops;

		void ReComputeProjAndFrustum();
	};
}

#endif
