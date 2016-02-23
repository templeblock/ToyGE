#pragma once
#ifndef MATH_H
#define MATH_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Vector.h"
#include "ToyGE\Math\Matrix.h"

#undef min
#undef max

namespace ToyGE
{
	using uint1 = Vector < uint32_t, 1 >;
	using uint2 = Vector < uint32_t, 2 >;
	using uint3 = Vector < uint32_t, 3 >;
	using uint4 = Vector < uint32_t, 4 >;
	using int1 = Vector < int32_t, 1 >;
	using int2 = Vector < int32_t, 2 >;
	using int3 = Vector < int32_t, 3 >;
	using int4 = Vector < int32_t, 4 >;
	using float1 = Vector < float, 1 >;
	using float2 = Vector < float, 2 >;
	using float3 = Vector < float, 3 >;
	using float4 = Vector < float, 4 >;

	template <typename T>
	using Matrix3 = Matrix<T, 3, 3>;
	template <typename T>
	using Matrix4 = Matrix<T, 4, 4>;

	using uint3x3 = Matrix3<uint32_t>;
	using int3x3 = Matrix3<int>;
	using float3x3 = Matrix3<float>;
	using uint4x4 = Matrix4<uint32_t>;
	using int4x4 = Matrix4<int>;
	using float4x4 = Matrix4<float>;

	enum BoundOverlap : uint32_t
	{
		BO_NO = 0UL,
		BO_PARTIAL = 1UL,
		BO_YES = 2UL
	};

	float const PI = 3.141592f;	
	float const PI2 = 6.283185f;			
	float const PI_DIV2 = 1.570796f;			

	float const DEG_90 = 1.570796f;			
	float const DEG_270 = -1.570796f;			
	float const DEG_45 = 0.7853981f;			
	float const DEG_5 = 0.0872664f;			
	float const DEG_10 = 0.1745329f;			
	float const DEG_20 = 0.3490658f;			
	float const DEG_30 = 0.5235987f;			
	float const DEG_60 = 1.047197f;			
	float const DEG_120 = 2.094395f;			

	float const DEG_40 = 0.6981317f;			
	float const DEG_80 = 1.396263f;			
	float const DEG_140 = 2.443460f;			
	float const DEG_160 = 2.792526f;			

	float const SQRT_2 = 1.414213f;			
	float const INV_SQRT_2 = 0.7071068f;			
	float const SQRT_3 = 1.732050f;			

	float const DEG2RAD = 0.01745329f;
	float const RAD2DEG = 57.29577f;
}

#include "ToyGE\Math\Quaternion.h"
#include "ToyGE\Math\Plane.h"
#include "ToyGE\Math\AABBox.h"
#include "ToyGE\Math\OBBox.h"
#include "ToyGE\Math\Sphere.h"
#include "ToyGE\Math\Frustum.h"

namespace ToyGE
{
	using Quaternion = Quaternion_T<float>;
	using AABBox = AABBox_T<float>;
	using OBBox = OBBox_T<float>;
	using Frustum = Frustum_T<float>;
	using Sphere = Sphere_T<float>;

	using std::abs;
	using std::sqrt;
	using std::log;
	using std::log2;
	using std::log10;
	using std::sin;
	using std::cos;
	using std::tan;
	using std::asin;
	using std::acos;
	using std::atan;
	using std::min;
	using std::max;
	using std::pow;

	// Common
	template <class T>
	T lerp(const T & v0, const T & v1, float x)
	{
		return T(v0 + (v1 - v0) * x);
	}

	template <class T>
	T clamp(const T & v, const T & minVal, float maxVal)
	{
		return min(maxVal, max(minVal, v));
	}

	template <typename T>
	inline bool
		equal(T const & lhs, T const & rhs)
	{
		return (lhs == rhs);
	}
	template <>
	inline bool
		equal<float>(float const & lhs, float const & rhs)
	{
		return (std::abs(lhs - rhs)
			<= std::numeric_limits<float>::epsilon());
	}
	template <>
	inline bool
		equal<double>(double const & lhs, double const & rhs)
	{
		return (abs(lhs - rhs)
			<= std::numeric_limits<double>::epsilon());
	}

	template <typename T>
	inline 
		T deg2rad(T const & x) 
	{
		return static_cast<T>(x * DEG2RAD);
	}
	template <typename T>
	inline 
		T rad2deg(T const & x) 
	{
		return static_cast<T>(x * RAD2DEG);
	}

	// Vector
	template <typename T, int N>
	Vector<T, N> operate2_vec(const Vector<T, N> & v0, const Vector<T, N> & v1, const std::function<T(const T&, const T&)> & func)
	{
		Vector<T, N> r;
		for (int i = 0; i < N; ++i)
			r[i] = func(v0[i], v1[i]);
		return r;
	}
	template <class T, int N>
	Vector<T, N> operate2_vec(const std::initializer_list<Vector<T, N>> & list, const std::function<T(const T&, const T&)> & func)
	{
		auto r = *list.begin();
		for (auto itr = list.begin() + 1; itr != list.end(); ++itr)
		{
			r = operate2_vec<T, N>(r, *itr, func);
		}
		return r;
	}

	template <typename T, int N>
	Vector<T, N> min_vec(const Vector<T, N> & v0, const Vector<T, N> & v1)
	{
		using std::min;
		
		return operate2_vec<T, N>(v0, v1, (const T&(*)(const T&, const T&))min);
	}
	template <class T, int N>
	Vector<T, N> min_vec(const std::initializer_list<Vector<T, N>> & list)
	{
		using std::min;

		return operate2_vec<T, N>(list, (const T&(*)(const T&, const T&))min);
	}

	template <typename T, int N>
	Vector<T, N> max_vec(const Vector<T, N> & v0, const Vector<T, N> & v1)
	{
		using std::max;

		return operate2_vec<T, N>(v0, v1, (const T&(*)(const T&, const T&))max);
	}
	template <class T, int N>
	Vector<T, N> max_vec(const std::initializer_list<Vector<T, N>> & list)
	{
		using std::max;

		return operate2_vec<T, N>(list, (const T&(*)(const T&, const T&))max);
	}


	template <typename T, int N>
	Vector<T, N> operate_vec(const Vector<T, N> & v, const std::function<T(const T&)> & func)
	{
		Vector<T, N> r;
		for (int i = 0; i < N; ++i)
			r[i] = func(v[i]);
		return r;
	}

	
	template <typename T, int N>
	Vector<T, N> abs(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, (T(const T&))abs);
	}

	template <typename T, int N>
	Vector<T, N> sqrt(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)sqrt);
	}

	template <typename T, int N>
	Vector<T, N> log(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)log);
	}

	template <typename T, int N>
	Vector<T, N> log2(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)log2);
	}

	template <typename T, int N>
	Vector<T, N> log10(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)log10);
	}

	template <typename T, int N>
	Vector<T, N> sin(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)sin);
	}

	template <typename T, int N>
	Vector<T, N> cos(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)cos);
	}

	template <typename T, int N>
	Vector<T, N> tan(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)tan);
	}

	template <typename T, int N>
	Vector<T, N> asin(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)asin);
	}

	template <typename T, int N>
	Vector<T, N> acos(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)acos);
	}

	template <typename T, int N>
	Vector<T, N> atan(const Vector<T, N> & v)
	{
		return operate_vec<T, N>(v, T(const T&)atan);
	}


	template <typename T, typename P, int N>
	Vector<T, N> pow(const Vector<T, N> & v, const Vector<P, N> & p)
	{
		Vector<T, N> r;
		for (int i = 0; i < N; ++i)
			r[i] = pow(v[i], p[i]);
		return r;
	}

	template <typename T>
	void sincos(const T & v, T & sinV, T & cosV)
	{
		sinV = sin(v);
		cosV = cos(v);
	}


	template <typename T, int N>
	T dot(const Vector<T, N> & v0, const Vector<T, N> & v1)
	{
		T r = v0[0] * v1[0];
		for (int i = 1; i < N; ++i)
			r += v0[i] * v1[i];
		return r;
	}

	template <typename T, int N>
	T length_sq(const Vector<T, N> & v)
	{
		return dot(v, v);
	}

	template <typename T, int N>
	T length(const Vector<T, N> & v)
	{
		return sqrt(length_sq(v));
	}

	template <typename T, int N>
	Vector<T, N> normalize(const Vector<T, N> & v)
	{
		const T l = length(v);
		return equal<T>(l, 0) ? v : v / l;
	}

	template <typename T>
	Plane_T<T> normalize(const Plane_T<T> & plane)
	{
		const T l = length(plane.normal());
		return equal<T>(l, 0) ? plane : plane / Plane_T<T>(l);
	}

	template <class T>
	Vector<T, 2> cross(const Vector<T, 2> & v0, const Vector<T, 2> & v1)
	{
		return Vector<T, 2>(v0.x() * v1.y() - v0.y() * v1.x());
	}

	template <class T>
	Vector<T, 3> cross(const Vector<T, 3> & v0, const Vector<T, 3> & v1)
	{
		return Vector<T, 3>
			(
			v0.y() * v1.z() - v0.z() * v1.y(),
			v0.z() * v1.x() - v0.x() * v1.z(),
			v0.x() * v1.y() - v0.y() * v1.x()
			);
	}

	template <int N>
	bool all(const Vector<bool, N> & v)
	{
		bool r = v[0];
		for (int i = 1; i < N; ++i)
			r = r && v[i];
		return r;
	}
	template <int N>
	bool any(const Vector<bool, N> & v)
	{
		bool r = v[0];
		for (int i = 1; i < N; ++i)
			r = r || v[i];
		return r;
	}

	template <int N>
	Vector<bool, N> and(const Vector<bool, N> & v0, const Vector<bool, N> & v1)
	{
		return operate2_vec<T, N>( v0, v1, [](bool b0, bool b1) {return b0 && b1; } );
	}

	template <int N>
	Vector<bool, N> or(const Vector<bool, N> & v0, const Vector<bool, N> & v1)
	{
		return operate2_vec<T, N>(v0, v1, [](bool b0, bool b1) {return b0 || b1; });
	}

	template <typename T>
	Vector<T, 4> mul(Vector<T, 4> const & v, Matrix4<T> const & mat)
	{
		return Vector<T, 4>(
			v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + v.w() * mat(3, 0),
			v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + v.w() * mat(3, 1),
			v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + v.w() * mat(3, 2),
			v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + v.w() * mat(3, 3));
	}

	template <typename T>
	Vector<T, 3> mul(Vector<T, 3> const & v, Matrix3<T> const & mat)
	{
		return Vector<T, 3>(
			v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0),
			v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1),
			v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2));
	}

	template <typename T>
	Plane_T<T> mul(Plane_T<T> const & v, Matrix4<T> const & mat)
	{
		const auto & invtransMat = transpose(inverse(mat));
		return Vector<T, 4>(
			v.x() * invtransMat(0, 0) + v.y() * invtransMat(1, 0) + v.z() * invtransMat(2, 0) + v.w() * invtransMat(3, 0),
			v.x() * invtransMat(0, 1) + v.y() * invtransMat(1, 1) + v.z() * invtransMat(2, 1) + v.w() * invtransMat(3, 1),
			v.x() * invtransMat(0, 2) + v.y() * invtransMat(1, 2) + v.z() * invtransMat(2, 2) + v.w() * invtransMat(3, 2),
			v.x() * invtransMat(0, 3) + v.y() * invtransMat(1, 3) + v.z() * invtransMat(2, 3) + v.w() * invtransMat(3, 3));
	}

	template <typename T>
	Vector<T, 3> transform_quat(Vector<T, 3> const & v, Quaternion_T<T> const & quat)
	{
		return v + cross(quat.v(), cross(quat.v(), v) + quat.w() * v) * T(2);
	}

	template <typename T>
	Vector<T, 3> transform_coord(Vector<T, 3> const & v, Matrix4<T> const & mat)
	{
		Vector<T, 4> temp(v.x(), v.y(), v.z(), 1);
		auto ret = mul<T>(temp, mat);
		if (equal<T>(temp.w(), 0))
		{
			ret = 0;
		}
		else
		{
			ret /= ret.w();
		}
		return Vector<T, 3>(ret.x(), ret.y(), ret.z());
	}

	template <typename T>
	Vector<T, 3> transform_normal(Vector<T, 3> const & v, Matrix4<T> const & mat)
	{
		Vector<T, 4> temp(v.x(), v.y(), v.z(), 0);
		auto ret = mul<T>(temp, mat);
		return Vector<T, 3>(ret.x(), ret.y(), ret.z());
	}

	// Matrix
	template <typename T, int N>
	Matrix<T, N, N> identity_mat()
	{
		Matrix<T, N, N> r(0);
		for (int i = 0; i < N; ++i)
			r[i][i] = 1;
		return r;
	}

	template <typename T, int N, int M>
	Matrix<T, M, N> transpose(const Matrix<T, N, M> & mat)
	{
		Matrix<T, M, N> r;
		for (int i = 0; i < M; ++i)
			for (int j = 0; j < N; ++j)
				r[i][j] = mat[j][i];
		return r;
	}

	template <typename T>
	Matrix4<T> transpose(Matrix4<T> const & rhs)
	{
		return Matrix4<T>(
		{
			rhs(0, 0), rhs(1, 0), rhs(2, 0), rhs(3, 0),
			rhs(0, 1), rhs(1, 1), rhs(2, 1), rhs(3, 1),
			rhs(0, 2), rhs(1, 2), rhs(2, 2), rhs(3, 2),
			rhs(0, 3), rhs(1, 3), rhs(2, 3), rhs(3, 3)
		});
	}

	template <typename T, int N, int M, int K>
	Matrix<T, N, K> mul(const Matrix<T, N, M> & lhs, const Matrix<T, M, K> & rhs)
	{
		Matrix<T, N, K> r;
		for (int i = 0; i < N; ++i)
			for (int j = 0; j < K; ++j)
				r[i][j] = dot(lhs.row(i), rhs.col(j));
		return r;
	}

	template <typename T>
	Matrix4<T> mul(const Matrix4<T> & lhs, const Matrix4<T> & rhs)
	{
		Matrix4<T> const tmp(transpose(rhs));

		return Matrix4<T>(
		{
			lhs(0, 0) * tmp(0, 0) + lhs(0, 1) * tmp(0, 1) + lhs(0, 2) * tmp(0, 2) + lhs(0, 3) * tmp(0, 3),
			lhs(0, 0) * tmp(1, 0) + lhs(0, 1) * tmp(1, 1) + lhs(0, 2) * tmp(1, 2) + lhs(0, 3) * tmp(1, 3),
			lhs(0, 0) * tmp(2, 0) + lhs(0, 1) * tmp(2, 1) + lhs(0, 2) * tmp(2, 2) + lhs(0, 3) * tmp(2, 3),
			lhs(0, 0) * tmp(3, 0) + lhs(0, 1) * tmp(3, 1) + lhs(0, 2) * tmp(3, 2) + lhs(0, 3) * tmp(3, 3),

			lhs(1, 0) * tmp(0, 0) + lhs(1, 1) * tmp(0, 1) + lhs(1, 2) * tmp(0, 2) + lhs(1, 3) * tmp(0, 3),
			lhs(1, 0) * tmp(1, 0) + lhs(1, 1) * tmp(1, 1) + lhs(1, 2) * tmp(1, 2) + lhs(1, 3) * tmp(1, 3),
			lhs(1, 0) * tmp(2, 0) + lhs(1, 1) * tmp(2, 1) + lhs(1, 2) * tmp(2, 2) + lhs(1, 3) * tmp(2, 3),
			lhs(1, 0) * tmp(3, 0) + lhs(1, 1) * tmp(3, 1) + lhs(1, 2) * tmp(3, 2) + lhs(1, 3) * tmp(3, 3),

			lhs(2, 0) * tmp(0, 0) + lhs(2, 1) * tmp(0, 1) + lhs(2, 2) * tmp(0, 2) + lhs(2, 3) * tmp(0, 3),
			lhs(2, 0) * tmp(1, 0) + lhs(2, 1) * tmp(1, 1) + lhs(2, 2) * tmp(1, 2) + lhs(2, 3) * tmp(1, 3),
			lhs(2, 0) * tmp(2, 0) + lhs(2, 1) * tmp(2, 1) + lhs(2, 2) * tmp(2, 2) + lhs(2, 3) * tmp(2, 3),
			lhs(2, 0) * tmp(3, 0) + lhs(2, 1) * tmp(3, 1) + lhs(2, 2) * tmp(3, 2) + lhs(2, 3) * tmp(3, 3),

			lhs(3, 0) * tmp(0, 0) + lhs(3, 1) * tmp(0, 1) + lhs(3, 2) * tmp(0, 2) + lhs(3, 3) * tmp(0, 3),
			lhs(3, 0) * tmp(1, 0) + lhs(3, 1) * tmp(1, 1) + lhs(3, 2) * tmp(1, 2) + lhs(3, 3) * tmp(1, 3),
			lhs(3, 0) * tmp(2, 0) + lhs(3, 1) * tmp(2, 1) + lhs(3, 2) * tmp(2, 2) + lhs(3, 3) * tmp(2, 3),
			lhs(3, 0) * tmp(3, 0) + lhs(3, 1) * tmp(3, 1) + lhs(3, 2) * tmp(3, 2) + lhs(3, 3) * tmp(3, 3)
		});
	}

	template <typename T>
	T determinant(const Matrix4<T> & rhs)
	{
		T const _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
		T const _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
		T const _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
		T const _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
		T const _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
		T const _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

		return rhs(0, 0) * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342)
			- rhs(0, 1) * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341)
			+ rhs(0, 2) * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241)
			- rhs(0, 3) * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241);
	}

	template <typename T>
	Matrix4<T> inverse(const Matrix4<T> & rhs)
	{
		T const _2132_2231(rhs(1, 0) * rhs(2, 1) - rhs(1, 1) * rhs(2, 0));
		T const _2133_2331(rhs(1, 0) * rhs(2, 2) - rhs(1, 2) * rhs(2, 0));
		T const _2134_2431(rhs(1, 0) * rhs(2, 3) - rhs(1, 3) * rhs(2, 0));
		T const _2142_2241(rhs(1, 0) * rhs(3, 1) - rhs(1, 1) * rhs(3, 0));
		T const _2143_2341(rhs(1, 0) * rhs(3, 2) - rhs(1, 2) * rhs(3, 0));
		T const _2144_2441(rhs(1, 0) * rhs(3, 3) - rhs(1, 3) * rhs(3, 0));
		T const _2233_2332(rhs(1, 1) * rhs(2, 2) - rhs(1, 2) * rhs(2, 1));
		T const _2234_2432(rhs(1, 1) * rhs(2, 3) - rhs(1, 3) * rhs(2, 1));
		T const _2243_2342(rhs(1, 1) * rhs(3, 2) - rhs(1, 2) * rhs(3, 1));
		T const _2244_2442(rhs(1, 1) * rhs(3, 3) - rhs(1, 3) * rhs(3, 1));
		T const _2334_2433(rhs(1, 2) * rhs(2, 3) - rhs(1, 3) * rhs(2, 2));
		T const _2344_2443(rhs(1, 2) * rhs(3, 3) - rhs(1, 3) * rhs(3, 2));
		T const _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
		T const _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
		T const _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
		T const _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
		T const _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
		T const _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

		T const det(determinant(rhs));
		if (equal<T>(det, static_cast<T>(0)))
		{
			return rhs;
		}
		else
		{
			T invDet(static_cast<T>(1) / det);

			return Matrix4<T>(
			{
				+invDet * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342),
				-invDet * (rhs(0, 1) * _3344_3443 - rhs(0, 2) * _3244_3442 + rhs(0, 3) * _3243_3342),
				+invDet * (rhs(0, 1) * _2344_2443 - rhs(0, 2) * _2244_2442 + rhs(0, 3) * _2243_2342),
				-invDet * (rhs(0, 1) * _2334_2433 - rhs(0, 2) * _2234_2432 + rhs(0, 3) * _2233_2332),

				-invDet * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341),
				+invDet * (rhs(0, 0) * _3344_3443 - rhs(0, 2) * _3144_3441 + rhs(0, 3) * _3143_3341),
				-invDet * (rhs(0, 0) * _2344_2443 - rhs(0, 2) * _2144_2441 + rhs(0, 3) * _2143_2341),
				+invDet * (rhs(0, 0) * _2334_2433 - rhs(0, 2) * _2134_2431 + rhs(0, 3) * _2133_2331),

				+invDet * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241),
				-invDet * (rhs(0, 0) * _3244_3442 - rhs(0, 1) * _3144_3441 + rhs(0, 3) * _3142_3241),
				+invDet * (rhs(0, 0) * _2244_2442 - rhs(0, 1) * _2144_2441 + rhs(0, 3) * _2142_2241),
				-invDet * (rhs(0, 0) * _2234_2432 - rhs(0, 1) * _2134_2431 + rhs(0, 3) * _2132_2231),

				-invDet * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241),
				+invDet * (rhs(0, 0) * _3243_3342 - rhs(0, 1) * _3143_3341 + rhs(0, 2) * _3142_3241),
				-invDet * (rhs(0, 0) * _2243_2342 - rhs(0, 1) * _2143_2341 + rhs(0, 2) * _2142_2241),
				+invDet * (rhs(0, 0) * _2233_2332 - rhs(0, 1) * _2133_2331 + rhs(0, 2) * _2132_2231)
			});
		}
	}

	template <typename T>
	Matrix4<T> look_at_lh(const Vector<T, 3> & vEye, const Vector<T, 3> & vAt, const Vector<T, 3> & vUp)
	{
		Vector<T, 3> zAxis(normalize(vAt - vEye));
		Vector<T, 3> xAxis(normalize(cross(vUp, zAxis)));
		Vector<T, 3> yAxis(cross(zAxis, xAxis));

		return Matrix4<T>(
		{
			xAxis.x(), yAxis.x(), zAxis.x(), static_cast<T>(0),
			xAxis.y(), yAxis.y(), zAxis.y(), static_cast<T>(0),
			xAxis.z(), yAxis.z(), zAxis.z(), static_cast<T>(0),
			-dot(xAxis, vEye), -dot(yAxis, vEye), -dot(zAxis, vEye), static_cast<T>(1)
		});
	}

	template <typename T>
	Matrix4<T> look_at_lh(Vector<T, 3> const & vEye, Vector<T, 3> const & vAt)
	{
		Vector<T, 3> vUp = abs(vAt.y() - vEye.y()) < static_cast<T>(1e-5) ? Vector<T, 3>(1, 0, 0) : Vector<T, 3>(0, 1, 0);
		return look_at_lh(vEye, vAt, vUp);
	}

	template <typename T>
	Matrix4<T> ortho_off_center_lh(T const & left, T const & right, T const & bottom, T const & top, T const & nearPlane_T, T const & farPlane_T)
	{
		T const q(T(1) / (farPlane_T - nearPlane_T));
		T const invWidth(T(1) / (right - left));
		T const invHeight(T(1) / (top - bottom));

		return Matrix4<T>(
		{
			invWidth + invWidth,		0,								0,					0,
			0,							invHeight + invHeight,			0,					0,
			0,							0,								q,					0,
			-(left + right) * invWidth,	-(top + bottom) * invHeight,	-nearPlane_T * q,		1
		});
	}

	template <typename T> 
	Matrix4<T> ortho_lh(const T & w, const T & h, const T & nearPlane_T, const T & farPlane_T)
	{
		T const w_2(w / 2);
		T const h_2(h / 2);
		return ortho_off_center_lh(-w_2, w_2, -h_2, h_2, nearPlane_T, farPlane_T);
	}
	
	template <typename T>
	Matrix4<T> perspective_lh(T const & width, T const & height, T const & nearPlane_T, T const & farPlane_T)
	{
		T const q(farPlane_T / (farPlane_T - nearPlane_T));
		T const near2(nearPlane_T + nearPlane_T);

		return Matrix4<T>(
		{
			near2 / width,	0,				0,				0,
			0,				near2 / height,	0,				0,
			0,				0,				q,				1,
			0,				0,				-nearPlane_T * q, 0
		});
	}

	template <typename T>
	Matrix4<T> perspective_fov_lh(T const & fov, T const & aspect, T const & nearPlane_T, T const & farPlane_T)
	{
		T const h(T(1) / tan(fov / 2));
		T const w(h / aspect);
		T const q(farPlane_T / (farPlane_T - nearPlane_T));

		return Matrix4<T>(
		{
			w,		0,		0,				0,
			0,		h,		0,				0,
			0,		0,		q,				1,
			0,		0,		-nearPlane_T * q, 0
		});
	}

	template <typename T>
	Matrix4<T> perspective_off_center_lh(T const & left, T const & right, T const & bottom, T const & top, T const & nearPlane_T, T const & farPlane_T)
	{
		T const q(farPlane_T / (farPlane_T - nearPlane_T));
		T const near2(nearPlane_T + nearPlane_T);
		T const invWidth(static_cast<T>(1) / (right - left));
		T const invHeight(static_cast<T>(1) / (top - bottom));

		return Matrix4<T>(
			near2 * invWidth, 0, 0, 0,
			0, near2 * invHeight, 0, 0,
			-(left + right) * invWidth, -(top + bottom) * invHeight, q, 1,
			0, 0, -nearPlane_T * q, 0);
	}

	template <typename T>
	Matrix4<T> lh_to_rh(Matrix4<T> const & rhs)
	{
		Matrix4<T> ret = rhs;
		ret(2, 0) = -ret(2, 0);
		ret(2, 1) = -ret(2, 1);
		ret(2, 2) = -ret(2, 2);
		ret(2, 3) = -ret(2, 3);
		return ret;
	}

	template <typename T>
	Matrix4<T> rh_to_lh(Matrix4<T> const & rhs)
	{
		return lh_to_rh(rhs);
	}

	template <typename T>
	Matrix4<T> rotation_x(T const & x) 
	{
		float sx, cx;
		sincos(x, sx, cx);

		return Matrix4<T>(
		{
			1, 0, 0, 0,
			0, cx, sx, 0,
			0, -sx, cx, 0,
			0, 0, 0, 1
		});
	}

	template <typename T>
	Matrix4<T> rotation_y(T const & y) 
	{
		float sy, cy;
		sincos(y, sy, cy);

		return Matrix4<T>(
		{
			cy, 0, -sy, 0,
			0, 1, 0, 0,
			sy, 0, cy, 0,
			0, 0, 0, 1
		});
	}

	template <typename T>
	Matrix4<T> rotation_z(T const & z) 
	{
		float sz, cz;
		sincos(z, sz, cz);

		return Matrix4<T>(
		{
			cz, sz, 0, 0,
			-sz, cz, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		});
	}

	template <typename T>
	Matrix4<T> rotation(T const & angle, T const & x, T const & y, T const & z) 
	{
		Quaternion_T<T> quat(rotation_axis(Vector<T, 3>(x, y, z), angle));
		return to_matrix(quat);
	}

	template <typename T>
	Matrix4<T> rotation_matrix_yaw_pitch_roll(T const & yaw, T const & pitch, T const & roll) 
	{
		Matrix4<T> rotX(rotation_x(pitch));
		Matrix4<T> rotY(rotation_y(yaw));
		Matrix4<T> rotZ(rotation_z(roll));
		return rotZ * rotX * rotY;
	}

	template <typename T>
	Matrix4<T> scaling(T const & sx, T const & sy, T const & sz) 
	{
		return Matrix4<T>(
		{
			sx, 0, 0, 0,
			0, sy, 0, 0,
			0, 0, sz, 0,
			0, 0, 0, 1
		});
	}

	template <typename T>
	Matrix4<T> scaling(Vector<T, 3> const & s) 
	{
		return scaling(s.x(), s.y(), s.z());
	}

	template <typename T>
	Matrix4<T> translation(T const & x, T const & y, T const & z) 
	{
		return Matrix4<T>(
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			x, y, z, 1
		});
	}

	template <typename T>
	Matrix4<T> translation(Vector<T, 3> const & pos) 
	{
		return translation(pos.x(), pos.y(), pos.z());
	}

	template <typename T>
	void decompose(Matrix4<T> const & rhs, Vector<T, 3>& scale, Quaternion_T<T>& rot, Vector<T, 3>& trans)
	{
		scale.x() = sqrt(rhs(0, 0) * rhs(0, 0) + rhs(0, 1) * rhs(0, 1) + rhs(0, 2) * rhs(0, 2));
		scale.y() = sqrt(rhs(1, 0) * rhs(1, 0) + rhs(1, 1) * rhs(1, 1) + rhs(1, 2) * rhs(1, 2));
		scale.z() = sqrt(rhs(2, 0) * rhs(2, 0) + rhs(2, 1) * rhs(2, 1) + rhs(2, 2) * rhs(2, 2));

		trans = Vector<T, 3>(rhs(3, 0), rhs(3, 1), rhs(3, 2));

		Matrix4<T> rot_mat;
		rot_mat(0, 0) = rhs(0, 0) / scale.x();
		rot_mat(0, 1) = rhs(0, 1) / scale.x();
		rot_mat(0, 2) = rhs(0, 2) / scale.x();
		rot_mat(0, 3) = 0;
		rot_mat(1, 0) = rhs(1, 0) / scale.y();
		rot_mat(1, 1) = rhs(1, 1) / scale.y();
		rot_mat(1, 2) = rhs(1, 2) / scale.y();
		rot_mat(1, 3) = 0;
		rot_mat(2, 0) = rhs(2, 0) / scale.z();
		rot_mat(2, 1) = rhs(2, 1) / scale.z();
		rot_mat(2, 2) = rhs(2, 2) / scale.z();
		rot_mat(2, 3) = 0;
		rot_mat(3, 0) = 0;
		rot_mat(3, 1) = 0;
		rot_mat(3, 2) = 0;
		rot_mat(3, 3) = 1;
		rot = to_quaternion(rot_mat);
	}

	template <typename T>
	Matrix4<T> transformation(Vector<T, 3> const * scaling_center, Quaternion_T<T> const * scaling_rotation, Vector<T, 3> const * scale,
		Vector<T, 3> const * rotation_center, Quaternion_T<T> const * rotation, Vector<T, 3> const * trans) 
	{
		Vector<T, 3> psc, prc, pt;
		if (scaling_center)
		{
			psc = *scaling_center;
		}
		else
		{
			psc = Vector<T, 3>(T(0), T(0), T(0));
		}
		if (rotation_center)
		{
			prc = *rotation_center;
		}
		else
		{
			prc = Vector<T, 3>(T(0), T(0), T(0));
		}
		if (trans)
		{
			pt = *trans;
		}
		else
		{
			pt = Vector<T, 3>(T(0), T(0), T(0));
		}

		Matrix4<T> m1, m2, m3, m4, m5, m6, m7;
		m1 = translation(-psc);
		if (scaling_rotation)
		{
			m4 = to_matrix(*scaling_rotation);
			m2 = inverse(m4);
		}
		else
		{
			m2 = m4 = identity_mat<T, 4>();
		}
		if (scale)
		{
			m3 = scaling(*scale);
		}
		else
		{
			m3 = identity_mat<T, 4>();
		}
		if (rotation)
		{
			m6 = to_matrix(*rotation);
		}
		else
		{
			m6 = identity_mat<T, 4>();
		}
		m5 = translation(psc - prc);
		m7 = translation(prc + pt);

		return m1 * m2 * m3 * m4 * m5 * m6 * m7;
	}

	// Quaternion_T
	template <typename T>
	Quaternion_T<T> identity_quat()
	{
		return Quaternion_T<T>(0, 0, 0, 1);
	}

	template <typename T>
	Quaternion_T<T> conjugate(Quaternion_T<T> const & rhs)
	{
		return Quaternion_T<T>(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
	}

	template <typename T>
	Quaternion_T<T> axis_to_axis(Vector<T, 3> const & from, Vector<T, 3> const & to)
	{
		Vector<T, 3> a(normalize(from));
		Vector<T, 3> b(normalize(to));

		return unit_axis_to_unit_axis(a, b);
	}

	template <typename T>
	Quaternion_T<T> unit_axis_to_unit_axis(Vector<T, 3> const & from, Vector<T, 3> const & to)
	{
		T const cos_theta = dot(from, to);
		if (equal<T>(cos_theta, T(1)))
		{
			return identity_quat<T>();
		}
		else
		{
			if (equal(cos_theta, T(-1)))
			{
				return Quaternion_T<T>(1, 0, 0, 0);
			}
			else
			{
				// From http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors

				Vector<T, 3> w = cross(from, to);
				return normalize(Quaternion_T<T>(w.x(), w.y(), w.z(), 1 + cos_theta));
			}
		}
	}

	template <typename T>
	Quaternion_T<T> inverse(Quaternion_T<T> const & rhs) 
	{
		T const inv(T(1) / length(rhs));
		return Quaternion_T<T>(-rhs.x() * inv, -rhs.y() * inv, -rhs.z() * inv, rhs.w() * inv);
	}

	template <typename T>
	Quaternion_T<T> mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs) 
	{
		return Quaternion_T<T>(
			lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
			lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
			lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
			lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
	}

	template <typename T>
	Quaternion_T<T> rotation_quat_yaw_pitch_roll(T const & yaw, T const & pitch, T const & roll) 
	{
		T const angX(pitch / 2), angY(yaw / 2), angZ(roll / 2);
		T sx, sy, sz;
		T cx, cy, cz;
		sincos(angX, sx, cx);
		sincos(angY, sy, cy);
		sincos(angZ, sz, cz);

		return Quaternion_T<T>(
			sx * cy * cz + cx * sy * sz,
			cx * sy * cz - sx * cy * sz,
			cx * cy * sz - sx * sy * cz,
			sx * sy * sz + cx * cy * cz);
	}

	// From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
	template <typename T>
	void to_yaw_pitch_roll(Quaternion_T<T> const & quat, T& yaw, T& pitch, T& roll)
	{
		T sqx = quat.x() * quat.x();
		T sqy = quat.y() * quat.y();
		T sqz = quat.z() * quat.z();
		T sqw = quat.w() * quat.w();
		T unit = sqx + sqy + sqz + sqw;
		T test = quat.w() * quat.x() + quat.y() * quat.z();
		if (test > T(0.499) * unit)
		{
			// singularity at north pole
			yaw = 2 * atan2(quat.z(), quat.w());
			pitch = PI / 2;
			roll = 0;
		}
		else
		{
			if (test < -T(0.499) * unit)
			{
				// singularity at south pole
				yaw = -2 * atan2(quat.z(), quat.w());
				pitch = -PI / 2;
				roll = 0;
			}
			else
			{
				yaw = atan2(2 * (quat.y() * quat.w() - quat.x() * quat.z()), -sqx - sqy + sqz + sqw);
				pitch = asin(2 * test / unit);
				roll = atan2(2 * (quat.z() * quat.w() - quat.x() * quat.y()), -sqx + sqy - sqz + sqw);
			}
		}
	}

	template <typename T>
	void to_axis_angle(Vector<T, 3>& vec, T& ang, Quaternion_T<T> const & quat) 
	{
		T const tw(acos(quat.w()));
		T const stw = sin(tw);

		ang = tw + tw;
		vec = quat.v();
		if (!equal<T>(stw, 0))
		{
			vec /= stw;
		}
	}

	

	template <typename T>
	Quaternion_T<T> rotation_axis(Vector<T, 3> const & v, T const & angle) 
	{
		T sa, ca;
		sincos(angle * T(0.5), sa, ca);

		if (equal<T>(length_sq(v), 0))
		{
			return Quaternion_T<T>(sa, sa, sa, ca);
		}
		else
		{
			return Quaternion_T<T>(sa * normalize(v), ca);
		}
	}

	template <typename T>
	Quaternion_T<T> slerp(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs, T s) 
	{
		T scale0, scale1;

		// DOT the quats to get the cosine of the angle between them
		T cosom = dot(lhs, rhs);

		T dir = T(1);
		if (cosom < 0)
		{
			dir = T(-1);
			cosom = -cosom;
		}

		// make sure they are different enough to avoid a divide by 0
		if (cosom < T(1) - std::numeric_limits<T>::epsilon())
		{
			// SLERP away
			T const omega = acos(cosom);
			T const isinom = T(1) / sin(omega);
			scale0 = sin((T(1) - s) * omega) * isinom;
			scale1 = sin(s * omega) * isinom;
		}
		else
		{
			// LERP is good enough at this distance
			scale0 = T(1) - s;
			scale1 = s;
		}

		// Compute the result
		return scale0 * lhs + dir * scale1 * rhs;
	}


	template <typename T>
	Quaternion_T<T> to_quaternion(Matrix4<T> const & mat)
	{
		Quaternion_T<T> quat;
		T s;
		T const tr = mat(0, 0) + mat(1, 1) + mat(2, 2) + 1;

		// check the diagonal
		if (tr > 1)
		{
			s = sqrt(tr);
			quat.w() = s * T(0.5);
			s = T(0.5) / s;
			quat.x() = (mat(1, 2) - mat(2, 1)) * s;
			quat.y() = (mat(2, 0) - mat(0, 2)) * s;
			quat.z() = (mat(0, 1) - mat(1, 0)) * s;
		}
		else
		{
			int maxi = 0;
			T maxdiag = mat(0, 0);
			for (int i = 1; i < 3; ++i)
			{
				if (mat(i, i) > maxdiag)
				{
					maxi = i;
					maxdiag = mat(i, i);
				}
			}

			switch (maxi)
			{
			case 0:
				s = sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

				quat.x() = s * T(0.5);

				if (!equal<T>(s, 0))
				{
					s = T(0.5) / s;
				}

				quat.w() = (mat(1, 2) - mat(2, 1)) * s;
				quat.y() = (mat(1, 0) + mat(0, 1)) * s;
				quat.z() = (mat(2, 0) + mat(0, 2)) * s;
				break;

			case 1:
				s = sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);
				quat.y() = s * T(0.5);

				if (!equal<T>(s, 0))
				{
					s = T(0.5) / s;
				}

				quat.w() = (mat(2, 0) - mat(0, 2)) * s;
				quat.z() = (mat(2, 1) + mat(1, 2)) * s;
				quat.x() = (mat(0, 1) + mat(1, 0)) * s;
				break;

			case 2:
			default:
				s = sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

				quat.z() = s * T(0.5);

				if (!equal<T>(s, 0))
				{
					s = T(0.5) / s;
				}

				quat.w() = (mat(0, 1) - mat(1, 0)) * s;
				quat.x() = (mat(0, 2) + mat(2, 0)) * s;
				quat.y() = (mat(1, 2) + mat(2, 1)) * s;
				break;
			}
		}

		return normalize(quat);
	}

	template <typename T>
	Quaternion_T<T> to_quaternion(Vector<T, 3> const & tangent, Vector<T, 3> const & binormal, Vector<T, 3> const & normal, int bits)
	{
		T k = 1;
		if (dot(binormal, cross(normal, tangent)) < 0)
		{
			k = -1;
		}

		Matrix4<T> tangent_frame(tangent.x(), tangent.y(), tangent.z(), 0,
			k * binormal.x(), k * binormal.y(), k * binormal.z(), 0,
			normal.x(), normal.y(), normal.z(), 0,
			0, 0, 0, 1);
		Quaternion_T<T> tangent_quat = to_quaternion(tangent_frame);
		if (tangent_quat.w() < 0)
		{
			tangent_quat = -tangent_quat;
		}
		if (bits > 0)
		{
			T const bias = T(1) / ((1UL << (bits - 1)) - 1);
			if (tangent_quat.w() < bias)
			{
				T const factor = sqrt(1 - bias * bias);
				tangent_quat.x() *= factor;
				tangent_quat.y() *= factor;
				tangent_quat.z() *= factor;
				tangent_quat.w() = bias;
			}
		}
		if (k < 0)
		{
			tangent_quat = -tangent_quat;
		}

		return tangent_quat;
	}

	template <typename T>
	Matrix4<T> to_matrix(Quaternion_T<T> const & quat)
	{
		// calculate coefficients
		T const x2(quat.x() + quat.x());
		T const y2(quat.y() + quat.y());
		T const z2(quat.z() + quat.z());

		T const xx2(quat.x() * x2), xy2(quat.x() * y2), xz2(quat.x() * z2);
		T const yy2(quat.y() * y2), yz2(quat.y() * z2), zz2(quat.z() * z2);
		T const wx2(quat.w() * x2), wy2(quat.w() * y2), wz2(quat.w() * z2);

		return Matrix4<T>(
		{
			1 - yy2 - zz2, xy2 + wz2, xz2 - wy2, 0,
			xy2 - wz2, 1 - xx2 - zz2, yz2 + wx2, 0,
			xz2 + wy2, yz2 - wx2, 1 - xx2 - yy2, 0,
			0, 0, 0, 1
		});
	}



	template <typename Iterator>
	AABBox_T<typename std::iterator_traits<Iterator>::value_type::value_type> compute_aabbox(Iterator first, Iterator last)
	{
		typedef typename std::iterator_traits<Iterator>::value_type::value_type value_type;

		Vector<value_type, 3> minVec = *first;
		Vector<value_type, 3> maxVec = *first;
		Iterator iter = first;
		++iter;
		for (; iter != last; ++iter)
		{
			Vector<value_type, 3> const & v = *iter;
			minVec = min_vec(minVec, v);
			maxVec = max_vec(maxVec, v);
		}
		return AABBox_T<value_type>(minVec, maxVec);
	}

	template <typename Iterator>
	OBBox_T<typename std::iterator_traits<Iterator>::value_type::value_type> compute_obbox(Iterator first, Iterator last)
	{
		typedef typename std::iterator_traits<Iterator>::value_type::value_type value_type;

		// Compute the mean of the points.
		Vector<value_type, 3> center = *first;
		Iterator iter = first;
		++iter;
		uint32_t n = 1;
		for (; iter != last; ++iter, ++n)
		{
			center += Vector<value_type, 3>(*iter);
		}
		value_type inv_num_points = value_type(1) / n;
		center *= inv_num_points;

		// Compute the covariance matrix of the points.
		value_type cov[6];
		for (int i = 0; i < 6; ++i)
		{
			cov[i] = 0;
		}

		for (iter = first; iter != last; ++iter)
		{
			Vector<value_type, 3> diff = Vector<value_type, 3>(*iter) - center;
			cov[0] += diff[0] * diff[0];
			cov[1] += diff[0] * diff[1];
			cov[2] += diff[0] * diff[2];
			cov[3] += diff[1] * diff[1];
			cov[4] += diff[1] * diff[2];
			cov[5] += diff[2] * diff[2];
		}

		for (int i = 0; i < 6; ++i)
		{
			cov[i] *= inv_num_points;
		}

		// Tridiagonal

		value_type diagonal[3];
		value_type sub_diagonal[3];
		value_type matrix[3][3];
		bool is_rotation = false;

		value_type m00 = cov[0];
		value_type m01 = cov[1];
		value_type m02 = cov[2];
		value_type m11 = cov[3];
		value_type m12 = cov[4];
		value_type m22 = cov[5];

		diagonal[0] = m00;
		sub_diagonal[2] = 0;
		if (abs(m02) > value_type(1e-6))
		{
			value_type length = sqrt(m01 * m01 + m02 * m02);
			value_type inv_length = 1 / length;
			m01 *= inv_length;
			m02 *= inv_length;
			value_type q = 2 * m01 * m12 + m02 * (m22 - m11);
			diagonal[1] = m11 + m02 * q;
			diagonal[2] = m22 - m02 * q;
			sub_diagonal[0] = length;
			sub_diagonal[1] = m12 - m01 * q;
			matrix[0][0] = 1;
			matrix[0][1] = 0;
			matrix[0][2] = 0;
			matrix[1][0] = 0;
			matrix[1][1] = m01;
			matrix[1][2] = m02;
			matrix[2][0] = 0;
			matrix[2][1] = m02;
			matrix[2][2] = -m01;
			is_rotation = false;
		}
		else
		{
			diagonal[1] = m11;
			diagonal[2] = m22;
			sub_diagonal[0] = m01;
			sub_diagonal[1] = m12;
			matrix[0][0] = 1;
			matrix[0][1] = 0;
			matrix[0][2] = 0;
			matrix[1][0] = 0;
			matrix[1][1] = 1;
			matrix[1][2] = 0;
			matrix[2][0] = 0;
			matrix[2][1] = 0;
			matrix[2][2] = 1;
			is_rotation = true;
		}

		// QLAlgorithm

		int const nIterPower = 32;

		for (int i0 = 0; i0 < 3; ++i0)
		{
			int i1;
			for (i1 = 0; i1 < nIterPower; ++i1)
			{
				int i2;
				for (i2 = i0; i2 <= 3 - 2; ++i2)
				{
					value_type tmp = abs(diagonal[i2]) + abs(diagonal[i2 + 1]);

					if (abs(sub_diagonal[i2]) + tmp == tmp)
					{
						break;
					}
				}
				if (i2 == i0)
				{
					break;
				}

				value_type value0 = (diagonal[i0 + 1] - diagonal[i0]) / (2 * sub_diagonal[i0]);
				value_type value1 = sqrt(value0 * value0 + 1);
				if (value0 < 0)
				{
					value0 = diagonal[i2] - diagonal[i0] + sub_diagonal[i0] / (value0 - value1);
				}
				else
				{
					value0 = diagonal[i2] - diagonal[i0] + sub_diagonal[i0] / (value0 + value1);
				}

				value_type sn = 1, cs = 1, value2 = 0;
				for (int i3 = i2 - 1; i3 >= i0; --i3)
				{
					value_type value3 = sn * sub_diagonal[i3];
					value_type value4 = cs * sub_diagonal[i3];
					if (abs(value3) >= abs(value0))
					{
						cs = value0 / value3;
						value1 = sqrt(cs * cs + 1);
						sub_diagonal[i3 + 1] = value3 * value1;
						sn = 1 / value1;
						cs *= sn;
					}
					else
					{
						sn = value3 / value0;
						value1 = sqrt(sn * sn + 1);
						sub_diagonal[i3 + 1] = value0 * value1;
						cs = 1 / value1;
						sn *= cs;
					}
					value0 = diagonal[i3 + 1] - value2;
					value1 = (diagonal[i3] - value0) * sn + 2 * value4 * cs;
					value2 = sn * value1;
					diagonal[i3 + 1] = value0 + value2;
					value0 = cs * value1 - value4;

					for (int i4 = 0; i4 < 3; ++i4)
					{
						value3 = matrix[i4][i3 + 1];
						matrix[i4][i3 + 1] = sn * matrix[i4][i3] + cs * value3;
						matrix[i4][i3] = cs * matrix[i4][i3] - sn * value3;
					}
				}
				diagonal[i0] -= value2;
				sub_diagonal[i0] = value0;
				sub_diagonal[i2] = 0;
			}
		}

		// IncreasingSort

		// Sort the eigenvalues in increasing order, e[0] <= ... <= e[mSize-1]
		for (int i0 = 0, i1; i0 <= 3 - 2; ++i0)
		{
			// Locate the minimum eigenvalue.
			i1 = i0;
			float min_value = diagonal[i1];
			int i2;
			for (i2 = i0 + 1; i2 < 3; ++i2)
			{
				if (diagonal[i2] < min_value)
				{
					i1 = i2;
					min_value = diagonal[i1];
				}
			}

			if (i1 != i0)
			{
				// Swap the eigenvalues.
				diagonal[i1] = diagonal[i0];
				diagonal[i0] = min_value;

				// Swap the eigenvectors corresponding to the eigenvalues.
				for (i2 = 0; i2 < 3; ++i2)
				{
					value_type tmp = matrix[i2][i0];
					matrix[i2][i0] = matrix[i2][i1];
					matrix[i2][i1] = tmp;
					is_rotation = !is_rotation;
				}
			}
		}

		// GuaranteeRotation

		if (!is_rotation)
		{
			// Change sign on the first column.
			for (int row = 0; row < 3; ++row)
			{
				matrix[row][0] = -matrix[row][0];
			}
		}

		Vector<value_type, 3> axis[3];
		Vector<value_type, 3> extent;
		for (int i = 0; i < 3; ++i)
		{
			extent[i] = diagonal[i];
			for (int row = 0; row < 3; ++row)
			{
				axis[i][row] = matrix[row][i];
			}
		}

		// Let C be the box center and let U0, U1, and U2 be the box axes.  Each
		// input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.  The
		// following code computes min(y0), max(y0), min(y1), max(y1), min(y2),
		// and max(y2).  The box center is then adjusted to be
		//   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1 +
		//        0.5*(min(y2)+max(y2))*U2

		Vector<value_type, 3> diff = Vector<value_type, 3>(*first) - center;
		Vector<value_type, 3> pmin(dot(diff, axis[0]), dot(diff, axis[1]), dot(diff, axis[2]));
		Vector<value_type, 3> pmax = pmin;
		iter = first;
		++iter;
		for (; iter != last; ++iter)
		{
			diff = Vector<value_type, 3>(*iter) - center;
			for (int j = 0; j < 3; ++j)
			{
				float d = dot(diff, axis[j]);
				if (d < pmin[j])
				{
					pmin[j] = d;
				}
				else if (d > pmax[j])
				{
					pmax[j] = d;
				}
			}
		}

		center += (value_type(0.5) * (pmin[0] + pmax[0])) * axis[0]
			+ (value_type(0.5) * (pmin[1] + pmax[1])) * axis[1]
			+ (value_type(0.5) * (pmin[2] + pmax[2])) * axis[2];

		extent[0] = value_type(0.5) * (pmax[0] - pmin[0]);
		extent[1] = value_type(0.5) * (pmax[1] - pmin[1]);
		extent[2] = value_type(0.5) * (pmax[2] - pmin[2]);

		return OBBox_T<value_type>(center, axis[0], axis[1], axis[2], extent);
	}

	template <typename Iterator>
	Sphere_T<typename std::iterator_traits<Iterator>::value_type::value_type> compute_sphere(Iterator first, Iterator last)
	{
		// from Graphics Gems I p301

		typedef typename std::iterator_traits<Iterator>::value_type::value_type value_type;

		value_type const min_float = std::numeric_limits<value_type>::min();
		value_type const max_float = std::numeric_limits<value_type>::max();
		Vector<value_type, 3> x_min(max_float, max_float, max_float);
		Vector<value_type, 3> y_min(max_float, max_float, max_float);
		Vector<value_type, 3> z_min(max_float, max_float, max_float);
		Vector<value_type, 3> x_max(min_float, min_float, min_float);
		Vector<value_type, 3> y_max(min_float, min_float, min_float);
		Vector<value_type, 3> z_max(min_float, min_float, min_float);
		for (Iterator iter = first; iter != last; ++iter)
		{
			if (x_min.x() > iter->x())
			{
				x_min = *iter;
			}
			if (y_min.y() > iter->y())
			{
				y_min = *iter;
			}
			if (z_min.z() > iter->z())
			{
				z_min = *iter;
			}

			if (x_max.x() < iter->x())
			{
				x_max = *iter;
			}
			if (y_max.y() < iter->y())
			{
				y_max = *iter;
			}
			if (z_max.z() < iter->z())
			{
				z_max = *iter;
			}
		}

		value_type x_span = length_sq(x_max - x_min);
		value_type y_span = length_sq(y_max - y_min);
		value_type z_span = length_sq(z_max - z_min);

		Vector<value_type, 3> dia1 = x_min;
		Vector<value_type, 3> dia2 = x_max;
		value_type max_span = x_span;
		if (y_span > max_span)
		{
			max_span = y_span;
			dia1 = y_min;
			dia2 = y_max;
		}
		if (z_span > max_span)
		{
			max_span = z_span;
			dia1 = z_min;
			dia2 = z_max;
		}

		Vector<value_type, 3> center((dia1 + dia2) / value_type(2));
		value_type r = length(dia2 - center);

		for (Iterator iter = first; iter != last; ++iter)
		{
			value_type d = length(Vector<value_type, 3>(*iter) - center);

			if (d > r)
			{
				r = (d + r) / 2;
				center = (r * center + (d - r) * Vector<value_type, 3>(*iter)) / d;
			}
		}

		return Sphere_T<value_type>(center, r);
	}

	template <typename T>
	Frustum_T<T> compute_frustum_from_clip(Matrix4<T> const & clip, Matrix4<T> const & inv_clip)
	{
		Frustum_T<T> frustum;

		frustum.corners[0] = transform_coord(Vector<T, 3>(-1, -1, 0), inv_clip); // left bottom near
		frustum.corners[1] = transform_coord(Vector<T, 3>(+1, -1, 0), inv_clip); // right bottom near
		frustum.corners[2] = transform_coord(Vector<T, 3>(-1, +1, 0), inv_clip); // left top near
		frustum.corners[3] = transform_coord(Vector<T, 3>(+1, +1, 0), inv_clip); // right top near
		frustum.corners[4] = transform_coord(Vector<T, 3>(-1, -1, 1), inv_clip); // left bottom far
		frustum.corners[5] = transform_coord(Vector<T, 3>(+1, -1, 1), inv_clip); // right bottom far
		frustum.corners[6] = transform_coord(Vector<T, 3>(-1, +1, 1), inv_clip); // left top far
		frustum.corners[7] = transform_coord(Vector<T, 3>(+1, +1, 1), inv_clip); // right top far

		Vector<T, 4> const & column1(clip.col(0));
		Vector<T, 4> const & column2(clip.col(1));
		Vector<T, 4> const & column3(clip.col(2));
		Vector<T, 4> const & column4(clip.col(3));

		frustum.planes[0] = column4 - column1;	// right
		frustum.planes[1] = column4 + column1;	// left
		frustum.planes[2] = column4 - column2;	// top
		frustum.planes[3] = column4 + column2;	// bottom
		frustum.planes[4] = column4 - column3;	// far
		frustum.planes[5] = column3;	// near

										// Loop through each side of the frustum and normalize it.
		for (auto& plane : frustum.planes)
		{
			plane = normalize(plane);
		}

		return frustum;
	}

	template <typename T>
	AABBox_T<T> convert_to_aabbox(OBBox_T<T> const & obb)
	{
		Vector<T, 3> min(+1e10f, +1e10f, +1e10f);
		Vector<T, 3> max(-1e10f, -1e10f, -1e10f);

		Vector<T, 3> const & center = obb.Center();
		Vector<T, 3> const & extent = obb.Extents();
		Vector<T, 3> const extent_x = extent.x() * obb.Axis(0);
		Vector<T, 3> const extent_y = extent.y() * obb.Axis(1);
		Vector<T, 3> const extent_z = extent.z() * obb.Axis(2);
		for (int i = 0; i < 8; ++i)
		{
			Vector<T, 3> corner = center + ((i & 1) ? extent_x : -extent_x)
				+ ((i & 2) ? extent_y : -extent_y) + ((i & 4) ? extent_z : -extent_z);

			min = minimize(min, corner);
			max = maximize(max, corner);
		}

		return AABBox_T<T>(min, max);
	}

	template <typename T>
	OBBox_T<T> convert_to_obbox(AABBox_T<T> const & aabb) 
	{
		return OBBox_T<T>(aabb, identity_quat<T>());
	}

	template <typename T>
	AABBox_T<T> transform_aabb(AABBox_T<T> const & aabb, Matrix4<T> const & mat)
	{
		Vector<T, 3> scale, trans;
		Quaternion_T<T> rot;
		decompose(mat, scale, rot, trans);

		return transform_aabb(aabb, scale, rot, trans);
	}

	template <typename T>
	AABBox_T<T> transform_aabb(AABBox_T<T> const & aabb, Vector<T, 3> const & scale, Quaternion_T<T> const & rot, Vector<T, 3> const & trans) 
	{
		Vector<T, 3> min, max;
		min = max = transform_quat(aabb.Corner(0) * scale, rot) + trans;
		for (int j = 1; j < 8; ++j)
		{
			Vector<T, 3> vec = transform_quat(aabb.Corner(j) * scale, rot) + trans;
			min = min_vec(min, vec);
			max = max_vec(max, vec);
		}

		return AABBox_T<T>(min, max);
	}

	template <typename T>
	OBBox_T<T> transform_obb(OBBox_T<T> const & obb, Matrix4<T> const & mat) 
	{
		Vector<T, 3> scale, trans;
		Quaternion_T<T> rot;
		decompose(mat, scale, rot, trans);

		return transform_obb(obb, scale, rot, trans);
	}

	template <typename T>
	OBBox_T<T> transform_obb(OBBox_T<T> const & obb, Vector<T, 3> const & scale, Quaternion_T<T> const & rot, Vector<T, 3> const & trans) 
	{
		Vector<T, 3> center = transform_quat(obb.Center() * scale, rot) + trans;
		Quaternion_T<T> rotation = mul(obb.Rotation(), rot);
		Vector<T, 3> extent = obb.Extents() * scale;
		return OBBox_T<T>(center, rotation, extent);
	}

	template <typename T>
	Sphere_T<T> transform_sphere(Sphere_T<T> const & sphere, Matrix4<T> const & mat) 
	{
		Vector<T, 3> scale, trans;
		Quaternion_T<T> rot;
		decompose(mat, scale, rot, trans);

		return transform_sphere(sphere, scale.x(), rot, trans);
	}

	template <typename T>
	Sphere_T<T> transform_sphere(Sphere_T<T> const & sphere, T scale, Quaternion_T<T> const & rot, Vector<T, 3> const & trans) 
	{
		Vector<T, 3> center = transform_quat(sphere.center * scale, rot) + trans;
		T radius = sphere.radius * scale;
		return Sphere_T<T>(center, radius);
	}

	template <typename T>
	Frustum_T<T> transform_frustum(Frustum_T<T> const & frustum, Matrix4<T> const & mat) 
	{
		Frustum_T<T> ret;
		for (int i = 0; i < 6; ++i)
		{
			ret.planes[i] = normalize(mul<T>(frustum.planes[i], mat));
		}
		for (int i = 0; i < 8; ++i)
		{
			ret.corners[i] = transform_coord(frustum.corners[i], mat);
		}

		return ret;
	}

	template <typename T>
	Frustum_T<T> transform_frustum(Frustum_T<T> const & frustum, const T & scale, Quaternion_T<T> const & rot, Vector<T, 3> const & trans) 
	{
		Vector<T, 3> vscale(scale, scale, scale);
		return transform_frustum(frustum, transformation<T>(nullptr, nullptr, &vscale, nullptr, &rot, &trans));
	}


	// Intersection
	template <typename T>
	T dot_plane(const Plane_T<T> & plane, const Vector<T, 3> & vec)
	{
		Vector<T, 4> tmp = Vector<T, 4>(vec.x(), vec.y(), vec.z(), T(1));
		return dot(plane.value, tmp);
	}

	template <typename T>
	bool intersect_point_aabb(Vector<T, 3> const & v, AABBox_T<T> const & aabb) 
	{
		return (in_bound(v.x(), aabb.min.x(), aabb.max.x()))
			&& (in_bound(v.y(), aabb.min.y(), aabb.max.y()))
			&& (in_bound(v.z(), aabb.min.z(), aabb.max.z()));
	}

	template <typename T>
	bool intersect_point_obb(Vector<T, 3> const & v, OBBox_T<T> const & obb) 
	{
		Vector<T, 3> d = v - obb.Center();
		return (dot(d, obb.Axis(0)) <= obb.Extents().x())
			&& (dot(d, obb.Axis(1)) <= obb.Extents().y())
			&& (dot(d, obb.Axis(2)) <= obb.Extents().z());
	}

	template <typename T>
	bool intersect_point_sphere(Vector<T, 3> const & v, Sphere_T<T> const & sphere) 
	{
		return length(v - sphere.center) < sphere.radius;
	}

	template <typename T>
	bool intersect_point_frustum(Vector<T, 3> const & v, Frustum_T<T> const & frustum) 
	{
		for (int i = 0; i < 6; ++i)
		{
			if (dot_plane(frustum.planes[i], v) < 0)
			{
				return false;
			}
		}
		return true;
	}


	template <typename T>
	bool intersect_ray_aabb(Vector<T, 3> const & orig, Vector<T, 3> const & dir, AABBox_T<T> const & aabb) 
	{
		T t_near = T(-1e10);
		T t_far = T(+1e10);

		for (int i = 0; i < 3; ++i)
		{
			if (equal(dir[i], T(0)))
			{
				if ((dir[i] < aabb.min[i]) || (dir[i] > aabb.max[i]))
				{
					return false;
				}
			}
			else
			{
				float t1 = (aabb.min[i] - orig[i]) / dir[i];
				float t2 = (aabb.max[i] - orig[i]) / dir[i];
				if (t1 > t2)
				{
					std::swap(t1, t2);
				}
				if (t1 > t_near)
				{
					t_near = t1;
				}
				if (t2 < t_far)
				{
					t_far = t2;
				}

				if (t_near > t_far)
				{
					// box is missed
					return false;
				}
				if (t_far < 0)
				{
					// box is behind ray
					return false;
				}
			}
		}

		return true;
	}

	template <typename T>
	bool intersect_ray_obb(Vector<T, 3> const & orig, Vector<T, 3> const & dir, OBBox_T<T> const & obb) 
	{
		T t_near = T(-1e10);
		T t_far = T(+1e10);

		Vector<T, 3> p = obb.Center() - orig;
		Vector<T, 3> const & extent = obb.Extents();
		for (int i = 0; i < 3; ++i)
		{
			T e = dot(obb.Axis(i), p);
			T f = dot(obb.Axis(i), dir);
			if (equal(f, T(0)))
			{
				if ((e < -extent[i]) || (e > extent[i]))
				{
					return false;
				}
			}
			else
			{
				float t1 = (e + extent[i]) / f;
				float t2 = (e - extent[i]) / f;
				if (t1 > t2)
				{
					std::swap(t1, t2);
				}
				if (t1 > t_near)
				{
					t_near = t1;
				}
				if (t2 < t_far)
				{
					t_far = t2;
				}

				if (t_near > t_far)
				{
					// box is missed
					return false;
				}
				if (t_far < 0)
				{
					// box is behind ray
					return false;
				}
			}
		}

		return true;
	}

	template <typename T>
	bool intersect_ray_sphere(Vector<T, 3> const & orig, Vector<T, 3> const & dir, Sphere_T<T> const & sphere) 
	{
		T const a = length_sq(dir);
		T const b = 2 * dot(dir, orig - sphere.center);
		T const c = length_sq(orig - sphere.center) - sphere.radius * sphere.radius;

		if (b * b - 4 * a * c < 0)
		{
			return false;
		}
		return true;
	}


	template <typename T>
	bool intersect_aabb_aabb(AABBox_T<T> const & lhs, AABBox_T<T> const & aabb) 
	{
		Vector<T, 3> const t = aabb.Center() - lhs.Center();
		Vector<T, 3> const e = aabb.Extents() + lhs.Extents();
		return (abs(t.x()) <= e.x()) && (abs(t.y()) <= e.y()) && (abs(t.z()) <= e.z());
	}

	template <typename T>
	bool intersect_aabb_obb(AABBox_T<T> const & lhs, OBBox_T<T> const & obb) 
	{
		return intersect_obb_obb(convert_to_obbox(lhs), obb);
	}

	template <typename T>
	bool intersect_aabb_sphere(AABBox_T<T> const & lhs, Sphere_T<T> const & sphere) 
	{
		Vector<T, 3> half_size = lhs.Extents();
		Vector<T, 3> d = sphere.center - lhs.Center();
		Vector<T, 3> closest_point_on_obb = lhs.Center();
		for (int i = 0; i < 3; ++i)
		{
			Vector<T, 3> axis(0, 0, 0);
			axis[i] = 1;
			T dist = dot(d, axis);
			if (dist > half_size[i])
			{
				dist = half_size[i];
			}
			if (dist < -half_size[i])
			{
				dist = -half_size[i];
			}
			closest_point_on_obb += dist * axis;
		}

		Vector<T, 3> v = closest_point_on_obb - sphere.center;
		return length_sq(v) <= sphere.radius * sphere.radius;
	}

	template <typename T>
	bool intersect_obb_obb(OBBox_T<T> const & lhs, OBBox_T<T> const & obb) 
	{
		// From Real-Time Collision Detection, p. 101-106. See http://realtimecollisiondetection.net/

		T epsilon = T(1e-3);

		Matrix4<T> r_mat = identity_mat<T, 4>();
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				r_mat(i, j) = dot(lhs.Axis(i), obb.Axis(j));
			}
		}

		Vector<T, 3> t = obb.Center() - lhs.Center();
		t = Vector<T, 3>(dot(t, lhs.Axis(0)), dot(t, lhs.Axis(1)), dot(t, lhs.Axis(2)));

		Matrix4<T> abs_r_mat = identity_mat<T, 4>();
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				abs_r_mat(i, j) = abs(r_mat(i, j)) + epsilon;
			}
		}

		Vector<T, 3> const & lr = lhs.Extents();
		Vector<T, 3> const & rr = obb.Extents();

		// Test the three major axes of this OBB.
		for (int i = 0; i < 3; ++i)
		{
			T ra = lr[i];
			T rb = rr[0] * abs_r_mat(i, 0) + rr[1] * abs_r_mat(i, 1) + rr[2] * abs_r_mat(i, 2);
			if (abs(t[i]) > ra + rb)
			{
				return false;
			}
		}

		// Test the three major axes of the OBB b.
		for (int i = 0; i < 3; ++i)
		{
			T ra = lr[0] * abs_r_mat(0, i) + lr[1] * abs_r_mat(1, i) + lr[2] * abs_r_mat(2, i);
			T rb = rr[i];
			if (abs(t.x() * r_mat(0, i) + t.y() * r_mat(1, i) + t.z() * r_mat(2, i)) > ra + rb)
			{
				return false;
			}
		}

		// Test the 9 different cross-axes.

		// A.x <cross> B.x
		T ra = lr.y() * abs_r_mat(2, 0) + lr.z() * abs_r_mat(1, 0);
		T rb = rr.y() * abs_r_mat(0, 2) + rr.z() * abs_r_mat(0, 1);
		if (abs(t.z() * r_mat(1, 0) - t.y() * r_mat(2, 0)) > ra + rb)
		{
			return false;
		}

		// A.x < cross> B.y
		ra = lr.y() * abs_r_mat(2, 1) + lr.z() * abs_r_mat(1, 1);
		rb = rr.x() * abs_r_mat(0, 2) + rr.z() * abs_r_mat(0, 0);
		if (abs(t.z() * r_mat(1, 1) - t.y() * r_mat(2, 1)) > ra + rb)
		{
			return false;
		}

		// A.x <cross> B.z
		ra = lr.y() * abs_r_mat(2, 2) + lr.z() * abs_r_mat(1, 2);
		rb = rr.x() * abs_r_mat(0, 1) + rr.y() * abs_r_mat(0, 0);
		if (abs(t.z() * r_mat(1, 2) - t.y() * r_mat(2, 2)) > ra + rb)
		{
			return false;
		}

		// A.y <cross> B.x
		ra = lr.x() * abs_r_mat(2, 0) + lr.z() * abs_r_mat(0, 0);
		rb = rr.y() * abs_r_mat(1, 2) + rr.z() * abs_r_mat(1, 1);
		if (abs(t.x() * r_mat(2, 0) - t.z() * r_mat(0, 0)) > ra + rb)
		{
			return false;
		}

		// A.y <cross> B.y
		ra = lr.x() * abs_r_mat(2, 1) + lr.z() * abs_r_mat(0, 1);
		rb = rr.x() * abs_r_mat(1, 2) + rr.z() * abs_r_mat(1, 0);
		if (abs(t.x() * r_mat(2, 1) - t.z() * r_mat(0, 1)) > ra + rb)
		{
			return false;
		}

		// A.y <cross> B.z
		ra = lr.x() * abs_r_mat(2, 2) + lr.z() * abs_r_mat(0, 2);
		rb = rr.x() * abs_r_mat(1, 1) + rr.y() * abs_r_mat(1, 0);
		if (abs(t.x() * r_mat(2, 2) - t.z() * r_mat(0, 2)) > ra + rb)
		{
			return false;
		}

		// A.z <cross> B.x
		ra = lr.x() * abs_r_mat(1, 0) + lr.y() * abs_r_mat(0, 0);
		rb = rr.y() * abs_r_mat(2, 2) + rr.z() * abs_r_mat(2, 1);
		if (abs(t.y() * r_mat(0, 0) - t.x() * r_mat(1, 0)) > ra + rb)
		{
			return false;
		}

		// A.z <cross> B.y
		ra = lr.x() * abs_r_mat(1, 1) + lr.y() * abs_r_mat(0, 1);
		rb = rr.x() * abs_r_mat(2, 2) + rr.z() * abs_r_mat(2, 0);
		if (abs(t.y() * r_mat(0, 1) - t.x() * r_mat(1, 1)) > ra + rb)
		{
			return false;
		}

		// A.z <cross> B.z
		ra = lr.x() * abs_r_mat(1, 2) + lr.y() * abs_r_mat(0, 2);
		rb = rr.x() * abs_r_mat(2, 1) + rr.y() * abs_r_mat(2, 0);
		if (abs(t.y() * r_mat(0, 2) - t.x() * r_mat(1, 2)) > ra + rb)
		{
			return false;
		}

		return true;
	}

	template <typename T>
	bool intersect_obb_sphere(OBBox_T<T> const & lhs, Sphere_T<T> const & sphere) 
	{
		Vector<T, 3> d = sphere.center - lhs.Center();
		Vector<T, 3> closest_point_on_obb = lhs.Center();
		for (int i = 0; i < 3; ++i)
		{
			T dist = dot(d, lhs.Axis(i));
			if (dist > lhs.Extents()[i])
			{
				dist = lhs.Extents()[i];
			}
			if (dist < -lhs.Extents()[i])
			{
				dist = -lhs.Extents()[i];
			}
			closest_point_on_obb += dist * lhs.Axis(i);
		}

		Vector<T, 3> v = closest_point_on_obb - sphere.center;
		return length_sq(v) <= sphere.radius * sphere.radius;
	}

	template <typename T>
	bool intersect_sphere_sphere(Sphere_T<T> const & lhs, Sphere_T<T> const & sphere) 
	{
		Vector<T, 3> d = lhs.center - sphere.center;
		float r = lhs.radius + sphere.radius;
		return length_sq(d) <= r * r;
	}


	template <typename T>
	BoundOverlap intersect_aabb_frustum(AABBox_T<T> const & aabb, Frustum_T<T> const & frustum) 
	{
		Vector<T, 3> const & min_pt = aabb.min;
		Vector<T, 3> const & max_pt = aabb.max;

		bool intersect = false;
		for (int i = 0; i < 6; ++i)
		{
			Plane_T<T> const & plane = frustum.planes[i];

			// v1 is diagonally opposed to v0
			Vector<T, 3> v0((plane.x() < 0) ? min_pt.x() : max_pt.x(), (plane.y() < 0) ? min_pt.y() : max_pt.y(), (plane.z() < 0) ? min_pt.z() : max_pt.z());
			Vector<T, 3> v1((plane.x() < 0) ? max_pt.x() : min_pt.x(), (plane.y() < 0) ? max_pt.y() : min_pt.y(), (plane.z() < 0) ? max_pt.z() : min_pt.z());

			if (dot_plane(plane, v0) < 0)
			{
				return BO_NO;
			}
			if (dot_plane(plane, v1) < 0)
			{
				intersect = true;
			}
		}

		return intersect ? BO_PARTIAL : BO_YES;
	}

	template <typename T>
	BoundOverlap intersect_obb_frustum(OBBox_T<T> const & obb, Frustum_T<T> const & frustum) 
	{
		Vector<T, 3> min_pt = obb.Corner(0);
		Vector<T, 3> max_pt = min_pt;
		for (int i = 1; i < 8; ++i)
		{
			Vector<T, 3> corner = obb.Corner(i);

			min_pt = minimize(min_pt, corner);
			max_pt = maximize(max_pt, corner);
		}

		bool intersect = false;
		for (int i = 0; i < 6; ++i)
		{
			Plane_T<T> const & plane = frustum.planes[i];

			// v1 is diagonally opposed to v0
			Vector<T, 3> v0((plane.x() < 0) ? min_pt.x() : max_pt.x(), (plane.y() < 0) ? min_pt.y() : max_pt.y(), (plane.z() < 0) ? min_pt.z() : max_pt.z());
			Vector<T, 3> v1((plane.x() < 0) ? max_pt.x() : min_pt.x(), (plane.y() < 0) ? max_pt.y() : min_pt.y(), (plane.z() < 0) ? max_pt.z() : min_pt.z());

			if (dot_plane(plane, v0) < 0)
			{
				return BO_NO;
			}
			if (dot_plane(plane, v1) < 0)
			{
				intersect = true;
			}
		}

		return intersect ? BO_PARTIAL : BO_YES;
	}

	template <typename T>
	BoundOverlap intersect_sphere_frustum(Sphere_T<T> const & sphere, Frustum_T<T> const & frustum) 
	{
		bool intersect = false;
		for (int i = 0; i < 6; ++i)
		{
			Plane_T<T> const & plane = frustum.planes[i];

			float d = dot_plane(plane, sphere.center);
			if (d <= -sphere.radius)
			{
				return BO_NO;
			}
			if (d > sphere.radius)
			{
				intersect = true;
			}
		}

		return intersect ? BO_PARTIAL : BO_YES;
	}

	template <typename T>
	BoundOverlap intersect_frustum_frustum(Frustum_T<T> const & lhs, Frustum_T<T> const & rhs) 
	{
		bool outside = false;
		bool inside_all = true;
		for (int i = 0; i < 6; ++i)
		{
			Plane_T<T> const & p = lhs.planes[i];

			T min_p, max_p;
			min_p = max_p = dot_plane(p, rhs.corners[0]);
			for (int j = 1; j < 8; ++j)
			{
				T tmp = dot_plane(p, rhs.Corner(j));
				min_p = std::min(min_p, tmp);
				max_p = std::max(max_p, tmp);
			}

			outside |= (min_p > 0);
			inside_all &= (max_p <= 0);
		}
		if (outside)
		{
			return BO_NO;
		}
		if (inside_all)
		{
			return BO_YES;
		}

		for (int i = 0; i < 6; ++i)
		{
			Plane_T<T> const & p = rhs.planes[i];

			T min_p = dot_plane(p, lhs.corners[0]);
			for (int j = 1; j < 8; ++j)
			{
				T tmp = dot_plane(p, lhs.corners[j]);
				min_p = std::min(min_p, tmp);
			}

			outside |= (min_p > 0);
		}
		if (outside)
		{
			return BO_NO;
		}

		Vector<T, 3> edge_axis_l[6];
		edge_axis_l[0] = rhs.corners[6];
		edge_axis_l[1] = rhs.corners[4];
		edge_axis_l[2] = rhs.corners[5];
		edge_axis_l[3] = rhs.corners[7];
		edge_axis_l[4] = rhs.corners[6] - rhs.corners[5];
		edge_axis_l[5] = rhs.corners[7] - rhs.corners[5];

		Vector<T, 3> edge_axis_r[6];
		edge_axis_r[0] = lhs.corners[6];
		edge_axis_r[1] = lhs.corners[4];
		edge_axis_r[2] = lhs.corners[5];
		edge_axis_r[3] = lhs.corners[7];
		edge_axis_r[4] = lhs.corners[6] - lhs.corners[5];
		edge_axis_r[5] = lhs.corners[7] - lhs.corners[5];

		for (int i = 0; i < 6; ++i)
		{
			for (int j = 0; j < 6; ++j)
			{
				Vector<T, 3> Axis = cross(edge_axis_l[i], edge_axis_r[j]);

				T min_l, max_l, min_r, max_r;
				min_l = max_l = dot(Axis, rhs.corners[0]);
				min_r = max_r = dot(Axis, lhs.corners[0]);
				for (int k = 1; k < 8; ++k)
				{
					T tmp = dot(Axis, rhs.corners[k]);
					min_l = std::min(min_l, tmp);
					max_l = std::max(max_l, tmp);

					tmp = dot(Axis, lhs.corners[k]);
					min_r = std::min(min_r, tmp);
					max_r = std::max(max_r, tmp);
				}

				outside |= min_l > max_r;
				outside |= min_r > max_l;
			}
		}
		if (outside)
		{
			return BO_NO;
		}

		return BO_PARTIAL;
	}


	template <typename T>
	void intersect(Vector<T, 3> const & v0, Vector<T, 3> const & v1, Vector<T, 3> const & v2,
		Vector<T, 3> const & ray_orig, Vector<T, 3> const & ray_dir,
		T& t, T& u, T& v) 
	{
		// Find vectors for two edges sharing vert0
		Vector<T, 3> edge1 = v1 - v0;
		Vector<T, 3> edge2 = v2 - v0;

		// Begin calculating determinant - also used to calculate U parameter
		Vector<T, 3> pvec(cross(ray_dir, edge2));

		// If determinant is near zero, ray lies in plane of triangle
		T det = dot(edge1, pvec);

		Vector<T, 3> tvec;
		if (det > 0)
		{
			tvec = ray_orig - v0;
		}
		else
		{
			tvec = v0 - ray_orig;
			det = -det;
		}

		// Calculate U parameter
		u = dot(tvec, pvec);

		// Prepare to test V parameter
		Vector<T, 3> qvec(cross(tvec, edge1));

		// Calculate V parameter
		v = dot(ray_dir, qvec);

		// Calculate t, scale parameters, ray intersects triangle
		t = dot(edge2, qvec);

		T const inv_det = T(1) / det;
		v *= inv_det;
		u *= inv_det;
		t *= inv_det;
	}
}



#endif