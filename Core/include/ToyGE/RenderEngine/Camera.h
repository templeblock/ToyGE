#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Kernel\StaticCastable.h"
#include "ToyGE\Math\Math.h"


namespace ToyGE
{

	class TOYGE_CORE_API Camera : public StaticCastable
	{
	public:
		Camera();

		virtual ~Camera() = default;

		void SetPos(const XMFLOAT3 & pos);
		CLASS_GET(Pos, XMFLOAT3, _pos);

		void SetViewMatrix(const XMFLOAT4X4 & matrix);
		const XMFLOAT4X4 & GetViewMatrix() const
		{
			return _viewMatrix;
		}

		virtual void SetProjMatrix(const XMFLOAT4X4 & matrix)
		{
			_projMatrix = matrix;
		}
		const XMFLOAT4X4 & GetProjMatrix() const
		{
			return _projMatrix;
		}

		XMFLOAT4X4 GetViewProjMatrix() const;

		void LookTo(const XMFLOAT3 & pos, const XMFLOAT3 & look, const XMFLOAT3 & up);

		void LookAt(const XMFLOAT3 & lookPos);

		const XMFLOAT3 & GetXAxis() const
		{
			return _xAxis;
		}
		const XMFLOAT3 & GetYAxis() const
		{
			return _yAxis;
		}
		const XMFLOAT3 & GetZAxis() const
		{
			return _zAxis;
		}

		virtual void SetNear(float nearDepth)
		{
			_nearDepth = nearDepth;
		}
		float GetNear() const
		{
			return _nearDepth;
		}

		virtual void SetFar(float farDepth)
		{
			_farDepth = farDepth;
		}
		float GetFar() const
		{
			return _farDepth;
		}

		virtual XNA::Frustum GetFrustum() const { return XNA::Frustum{ 0 }; };

		void Walk(float value);
		void Strafe(float value);
		void Fly(float value);

		void Rotate(const XMFLOAT3 & axis, float angle);
		void Yaw(float angle);
		void Pitch(float angle);
		void Roll(float angle);

		CLASS_GET(ViewMatrixCache, XMFLOAT4X4, _viewMatrixCache);
		CLASS_SET(ViewMatrixCache, XMFLOAT4X4, _viewMatrixCache);

		virtual void Cull(const Ptr<class SceneCuller> & culler, std::vector<Ptr<class Cullable>> & outElements) = 0;

	protected:
		XMFLOAT3 _pos;
		XMFLOAT3 _xAxis;
		XMFLOAT3 _yAxis;
		XMFLOAT3 _zAxis;
		XMFLOAT4X4 _viewMatrix;
		XMFLOAT4X4 _viewMatrixCache;
		XMFLOAT4X4 _projMatrix;
		float _nearDepth;
		float _farDepth;

		void UpdateViewMatrix();
	};

	class TOYGE_CORE_API PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(float fovAngle, float aspectRatio, float nearZ, float farZ);

		virtual void SetProjMatrix(const XMFLOAT4X4 & matrix) override;

		virtual XNA::Frustum GetFrustum() const override;

		virtual void SetNear(float nearDepth) override;

		virtual void SetFar(float farDepth) override;

		void SetFovAngle(float angle);
		virtual float GetFovAngle() const
		{
			return _fovAngle;
		}

		void SetAspectRatio(float ratio);
		virtual float GetAspectRatio() const
		{
			return _aspectRatio;
		}

		virtual void Cull(const Ptr<class SceneCuller> & culler, std::vector<Ptr<class Cullable>> & outElements) override;

	protected:
		XNA::Frustum _frustum;
		float _fovAngle;
		float _aspectRatio;

		void UpdateProjMatrix();
	};

	class TOYGE_CORE_API OrthogonalCamera : public Camera
	{
	public:
		OrthogonalCamera();

		virtual void SetProjMatrix(const XMFLOAT4X4 & matrix) override;

		virtual void SetNear(float nearDepth) override;

		virtual void SetFar(float farDepth) override;

		void SetViewBox(float left, float right, float bottom, float up, float front, float back);

		virtual void Cull(const Ptr<class SceneCuller> & culler, std::vector<Ptr<class Cullable>> & outElements) override;

	protected:
		float _left = 0.0f;
		float _right = 0.0f;
		float _bottom = 0.0f;
		float _up = 0.0f;

		void UpdateProjMatrix();
	};

	//class TOYGE_CORE_API PhysicalCamera : public PerspectiveCamera
	//{
	//public:
	//	PhysicalCamera(float nearZ, float farZ);

	//	float FovAngle() const override
	//	{
	//		float v = _focalDistance * _focalLength / (_focalDistance - _focalLength);
	//		return std::atan(_fimSize.y / (2.0f * v)) * 2.0f;
	//	}

	//	float AspectRatio() const override
	//	{
	//		return _fimSize.x / _fimSize.y;
	//	}

	//	void SetFilmSize(const float2 & filmSize)
	//	{
	//		_fimSize = filmSize;
	//		ReComputeProjAndFrustum();
	//	}

	//	const float2 & GetFilmSize() const
	//	{
	//		return _fimSize;
	//	}

	//	void SetFocalLength(float focalLength)
	//	{
	//		_focalLength = focalLength;
	//		ReComputeProjAndFrustum();
	//	}

	//	const float & GetFocalLength() const
	//	{
	//		return _focalLength;
	//	}

	//	void SetFocalDistance(float focalDistance)
	//	{
	//		_focalDistance = focalDistance;
	//		ReComputeProjAndFrustum();
	//	}

	//	const float & GetFocalDistance() const
	//	{
	//		return _focalDistance;
	//	}

	//	void SetFStops(float fstops)
	//	{
	//		_fstops = fstops;
	//	}

	//	const float & GetFStops() const
	//	{
	//		return _fstops;
	//	}

	//private:
	//	//XMFLOAT4X4 _projMatrix;
	//	//XNA::Frustum _frustum;
	//	//float _fovAngle;
	//	//float _aspectRatio;
	//	float2 _fimSize;
	//	float _focalLength;
	//	float _focalDistance;
	//	float _fstops;

	//	void ReComputeProjAndFrustum();
	//};
}

#endif
