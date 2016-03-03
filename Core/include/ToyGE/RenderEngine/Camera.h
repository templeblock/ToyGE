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

		void SetViewMatrix(const float4x4 & matrix);
		const float4x4 & GetViewMatrix() const
		{
			return _viewMatrix;
		}

		virtual void SetProjMatrix(const float4x4 & matrix);
		const float4x4 & GetProjMatrix() const
		{
			return _projMatrix;
		}

		float4x4 GetViewProjMatrix() const;

		void SetPos(const float3 & pos);
		CLASS_GET(Pos, float3, _pos);

		void LookTo(const float3 & pos, const float3 & look, const float3 & up);

		void LookAt(const float3 & lookPos);

		const float3 & GetXAxis() const
		{
			return _xAxis;
		}
		const float3 & GetYAxis() const
		{
			return _yAxis;
		}
		const float3 & GetZAxis() const
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

		virtual Frustum GetFrustum() const { return _frustum; };

		void Walk(float value);
		void Strafe(float value);
		void Fly(float value);

		void Rotate(const float3 & axis, float angle);
		void Yaw(float angle);
		void Pitch(float angle);
		void Roll(float angle);

		CLASS_GET(ViewMatrixCache, float4x4, _viewMatrixCache);
		CLASS_SET(ViewMatrixCache, float4x4, _viewMatrixCache);

		CLASS_GET(ProjMatrixCache, float4x4, _projMatrixCache);
		CLASS_SET(ProjMatrixCache, float4x4, _projMatrixCache);

		virtual void Cull(const Ptr<class SceneCuller> & culler, std::vector<Ptr<class Cullable>> & outElements);

	protected:
		float3 _pos;
		float3 _xAxis;
		float3 _yAxis;
		float3 _zAxis;
		float4x4 _viewMatrix;
		float4x4 _projMatrix;
		float4x4 _viewProjMatrix;
		float4x4 _viewMatrixCache;
		float4x4 _projMatrixCache;
		float _nearDepth;
		float _farDepth;

		Frustum _frustum;

		void UpdateViewMatrix();

		void UpdateFrustum();
	};

	class TOYGE_CORE_API PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera();

		PerspectiveCamera(float fovAngle, float aspectRatio, float nearZ, float farZ);

		virtual void SetProjMatrix(const float4x4 & matrix) override;

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

	protected:
		float _fovAngle;
		float _aspectRatio;
	};

	class TOYGE_CORE_API OrthogonalCamera : public Camera
	{
	public:
		OrthogonalCamera();

		OrthogonalCamera(float left, float right, float bottom, float up, float front, float back);

		virtual void SetProjMatrix(const float4x4 & matrix) override;

		virtual void SetNear(float nearDepth) override;

		virtual void SetFar(float farDepth) override;

		void SetViewBox(float left, float right, float bottom, float up, float front, float back);

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
	//	//float4x4 _projMatrix;
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
