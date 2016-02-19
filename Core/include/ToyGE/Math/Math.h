#pragma once
#ifndef MATH_H
#define MATH_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Vector.h"
#include "ToyGE\Math\Matrix.h"
#include "Externals\xnacollision\xnamath.h"
#include "Externals\xnacollision\xnacollision.h"

namespace ToyGE
{
	// Common
	template <class T>
	T lerp(const T & v0, const T & v1, float x)
	{
		return T(v0 * (1.0f - x) + v1 * x);
	}

	// Vector
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

	using std::abs;
	template <typename T, int N>
	Vector<T, N> abs(const Vector<T, N> & v)
	{
		return operate_vec(v, abs);
	}

	using std::sqrt;
	template <typename T, int N>
	Vector<T, N> sqrt(const Vector<T, N> & v)
	{
		return operate_vec(v, sqrt);
	}

	using std::log;
	template <typename T, int N>
	Vector<T, N> log(const Vector<T, N> & v)
	{
		return operate_vec(v, log);
	}

	using std::log2;
	template <typename T, int N>
	Vector<T, N> log2(const Vector<T, N> & v)
	{
		return operate_vec(v, log2);
	}

	using std::log10;
	template <typename T, int N>
	Vector<T, N> log10(const Vector<T, N> & v)
	{
		return operate_vec(v, log10);
	}

	using std::pow;
	template <typename T, typename P, int N>
	Vector<T, N> pow(const Vector<T, N> & v, const Vector<P, N> & p)
	{
		Vector<T, N> r;
		for (int i = 0; i < N; ++i)
			r[i] = pow(v[i], p[i]);
		return r;
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
	T length(const Vector<T, N> & v)
	{
		return sqrt(dot(v, v));
	}

	template <typename T, int N>
	Vector<T, N> normalize(const Vector<T, N> & v)
	{
		float l = length(v);
		return l == 0.0f ? 0.0f : v / l;
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

	//template <class T>
	//Vector4<T> mul(const Vector4<T> & vec, const Matrix4<T> & mat)
	//{
	//	return Vector4 <T>
	//		(
	//		vec[0] * mat[0][0] + vec[1] * mat[1][0] + vec[2] * mat[2][0] + vec[3] * mat[3][0],
	//		vec[0] * mat[0][1] + vec[1] * mat[1][1] + vec[2] * mat[2][1] + vec[3] * mat[3][1],
	//		vec[0] * mat[0][2] + vec[1] * mat[1][2] + vec[2] * mat[2][2] + vec[3] * mat[3][2],
	//		vec[0] * mat[0][3] + vec[1] * mat[1][3] + vec[2] * mat[2][3] + vec[3] * mat[3][3],
	//		);
	//}

	//template <class T>
	//Matrix4<T> mul(const Matrix4<T> & mat0, const Matrix4<T> & mat1)
	//{
	//	return Matrix4 <T>
	//		(
	//		mul(mat0[0], mat1),
	//		mul(mat0[1], mat1),
	//		mul(mat0[2], mat1),
	//		mul(mat0[3], mat1)
	//		);
	//}

	//template <class T>
	//T det(const Matrix4<T> & mat)
	//{
	//	const T _2233_3223 = mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3];
	//	const T _2133_3123 = mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3];
	//	const T _2132_3122 = mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2];
	//	const T _2033_3023 = mat[2][0] * mat[3][3] - mat[3][0] * mat[2][3];
	//	const T _2032_3022 = mat[2][0] * mat[3][2] - mat[3][0] * mat[2][2];
	//	const T _2031_3021 = mat[2][0] * mat[3][1] - mat[3][0] * mat[2][1];

	//	//const T M00_11 = _2233_3223;
	//	//const T M00_12 = _2133_3123;
	//	//const T M00_13 = _2132_3122;
	//	const T M00 = mat[1][1] * _2233_3223 - mat[1][2] * _2133_3123 + mat[1][3] * _2132_3122;

	//	//const T M01_10 = _2233_3223;
	//	//const T M01_12 = _2033_3023;
	//	//const T M01_13 = _2032_3022;
	//	const T M01 = mat[1][0] * _2233_3223 - mat[1][2] * _2033_3023 + mat[1][3] * _2032_3022;

	//	//const T M02_10 = _2133_3123;
	//	//const T M02_11 = _2033_3023;
	//	//const T M02_13 = _2031_3021;
	//	const T M02 = mat[1][0] * _2133_3123 - mat[1][1] * _2033_3023 + mat[1][3] * _2031_3021;

	//	//const T M03_10 = _2132_3122;
	//	//const T M03_11 = _2032_3022;
	//	//const T M03_12 = _2031_3021;
	//	const T M03 = mat[1][0] * _2132_3122 - mat[1][1] * _2032_3022 + mat[1][2] * _2031_3021;

	//	return mat[0][0] * M00 - mat[0][1] * M01 + mat[0][2] * M02 - mat[0][3] * M03;
	//}

	class TOYGE_CORE_API Math
	{
	public:
		static void AxisAlignedBoxToMinMax(const XNA::AxisAlignedBox & aabb, float3 & min, float3 & max)
		{
			min.x() = aabb.Center.x - aabb.Extents.x;
			min.y() = aabb.Center.y - aabb.Extents.y;
			min.z() = aabb.Center.z - aabb.Extents.z;

			max.x() = aabb.Center.x + aabb.Extents.x;
			max.y() = aabb.Center.y + aabb.Extents.y;
			max.z() = aabb.Center.z + aabb.Extents.z;
		}

		static void MinMaxToAxisAlignedBox(const float3 & min, const float3 & max, XNA::AxisAlignedBox & aabb)
		{
			float3 center = (min + max) * 0.5f;
			float3 extents = (max - min) * 0.5f;
			aabb.Center = *(reinterpret_cast<XMFLOAT3*>(&center));
			aabb.Extents = *(reinterpret_cast<XMFLOAT3*>(&extents));
		}

		template <typename T>
		static T Calculate(const String & str)
		{
			std::stack<T> numStack;
			std::stack<char> symbolStack;

			symbolStack.push('#');

			for (int i = 0; i < static_cast<int>(str.size()); ++i)
			{
				auto ch = str[i];
				switch (ch)
				{
				case ' ':
					continue;

				case '#':
				case '(':
				case '+':
				case '-':
				case '*':
				case '/':
				{
					if (GetPriority(ch) >= GetPriority(symbolStack.top()))
					{
						symbolStack.push(ch);
					}
					else
					{
						while (GetPriority(ch) < GetPriority(symbolStack.top()))
						{
							T v0 = numStack.top();
							numStack.pop();
							T v1 = numStack.top();
							numStack.pop();
							char op = symbolStack.top();
							symbolStack.pop();
							T v = ExcuteCalculate(v1, v0, op);
							numStack.push(v);
						}
						symbolStack.push(ch);
					}
				}
				break;

				case ')':
				{
					while (symbolStack.top() != '(')
					{
						T v0 = numStack.top();
						numStack.pop();
						T v1 = numStack.top();
						numStack.pop();
						char op = symbolStack.top();
						symbolStack.pop();
						T v = ExcuteCalculate(v1, v0, op);
						numStack.push(v);
					}
					symbolStack.pop();
				}
				break;

				default:
				{
					int begin = i;
					int end = begin + 1;
					while (end < static_cast<int>(str.size()) && str[end] >= '0' && str[end] <= '9')
						++end;
					T v = std::stoi(str.substr(begin, end - begin));
					numStack.push(v);
					i = end - 1;
				}
				break;
				}
			}

			while (symbolStack.top() != '#')
			{
				T v0 = numStack.top();
				numStack.pop();
				T v1 = numStack.top();
				numStack.pop();
				char op = symbolStack.top();
				symbolStack.pop();
				T v = ExcuteCalculate(v1, v0, op);
				numStack.push(v);
			}

			T result = numStack.top();

			return result;
		}

	private:
		static int GetPriority(char op)
		{
			int prio = -1;
			switch (op)
			{
			case '#':
				prio = 0;
				break;

			case '(':
				prio = 1;
				break;

			case '+':
			case '-':
				prio = 2;
				break;

			case '*':
			case '/':
				prio = 3;
				break;

			default:
				break;
			}

			return prio;
		}

		template <typename T>
		static T ExcuteCalculate(T v0, T v1, char op)
		{
			switch (op)
			{
			case '+':
				return v0 + v1;
			case '-':
				return v0 - v1;
			case '*':
				return v0 * v1;
			case '/':
				return v1 == 0 ? 0 : v0 / v1;
			default:
				break;
			}
			return 0;
		}
	};
}

#endif