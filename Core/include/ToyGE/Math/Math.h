#pragma once
#ifndef MATH_H
#define MATH_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Math\Vector.h"
#include "ToyGE\Math\Matrix.h"
#include "Externals\xnacollision\xnamath.h"
#include "Externals\xnacollision\xnacollision.h"

namespace ToyGE
{
	using uint2 = Vector2 < uint32_t > ;
	using uint3 = Vector3 < uint32_t >;
	using uint4 = Vector4 < uint32_t >;
	using int2 = Vector2 < int32_t >;
	using int3 = Vector3 < int32_t >;
	using int4 = Vector4 < int32_t >;
	using float2 = Vector2 < float >;
	using float3 = Vector3 < float >;
	using float4 = Vector4 < float >;

	using uint4x4 = Matrix4 < uint32_t >;
	using int4x4 = Matrix4 < int32_t >;
	using float4x4 = Matrix4 < float >;

	template <class T>
	Vector2<T> vecMin(const Vector2<T> & v0, const Vector2<T> & v1)
	{
		return Vector2<T>(std::min<T>(v0.x, v1.x), std::min<T>(v0.y, v1.y));
	}

	template <class T>
	Vector3<T> vecMin(const Vector3<T> & v0, const Vector3<T> & v1)
	{
		return Vector3<T>(std::min<T>(v0.x, v1.x), std::min<T>(v0.y, v1.y), std::min<T>(v0.z, v1.z));
	}

	template <class T>
	Vector4<T> vecMin(const Vector4<T> & v0, const Vector4<T> & v1)
	{
		return Vector4<T>(std::min<T>(v0.x, v1.x), std::min<T>(v0.y, v1.y), std::min<T>(v0.z, v1.z), std::min<T>(v0.w, v1.w));
	}

	template <class T>
	T vecMin(const std::initializer_list<T> & list)
	{
		auto min = *list.begin();
		for (auto itr = list.begin() + 1; itr != list.end(); ++itr)
		{
			min = vecMin(min, *itr);
		}
		return min;
	}

	template <class T>
	Vector2<T> vecMax(const Vector2<T> & v0, const Vector2<T> & v1)
	{
		return Vector2<T>(std::max<T>(v0.x, v1.x), std::max<T>(v0.y, v1.y));
	}

	template <class T>
	Vector3<T> vecMax(const Vector3<T> & v0, const Vector3<T> & v1)
	{
		return Vector3<T>(std::max<T>(v0.x, v1.x), std::max<T>(v0.y, v1.y), std::max<T>(v0.z, v1.z));
	}

	template <class T>
	Vector4<T> vecMax(const Vector4<T> & v0, const Vector4<T> & v1)
	{
		return Vector4<T>(std::max<T>(v0.x, v1.x), std::max<T>(v0.y, v1.y), std::max<T>(v0.z, v1.z), std::max<T>(v0.w, v1.w));
	}

	template <class T>
	T vecMax(const std::initializer_list<T> & list)
	{
		auto max = *list.begin();
		for (auto itr = list.begin() + 1; itr != list.end(); ++itr)
		{
			max = vecMax(max, *itr);
		}
		return max;
	}

	template <class T>
	Vector2<T> vecAbs(const Vector2<T> & v)
	{
		return Vector2<T>(abs(v.x), abs(v.y));
	}

	template <class T>
	Vector3<T> vecAbs(const Vector3<T> & v)
	{
		return Vector3<T>(abs(v.x), abs(v.y), abs(v.z));
	}

	template <class T>
	Vector4<T> vecAbs(const Vector4<T> & v)
	{
		return Vector4<T>(abs(v.x), abs(v.y), abs(v.z), abs(v.w));
	}

	template <class T>
	T lerp(const T & v0, const T & v1, float x)
	{
		return T( v0 * (1.0f - x) + v1 * x );
	}


	template <class T>
	T dot(const Vector2<T> & lhs, const Vector2<T> & rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	template <class T>
	T dot(const Vector3<T> & lhs, const Vector3<T> & rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	template <class T>
	T dot(const Vector4<T> & lhs, const Vector4<T> & rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}

	template <class T>
	Vector2<T> cross(const Vector2<T> & v0, const Vector2<T> & v1)
	{
		return Vector2 <T>(v0.x * v1.y - v0.y * v1.x);
	}

	template <class T>
	Vector3<T> cross(const Vector3<T> & v0, const Vector3<T> & v1)
	{
		return Vector3 <T>
			(
			v0.y * v1.z - v0.z * v1.y,
			v0.z * v1.x - v0.x * v1.z,
			v0.x * v1.y - v0.y * v1.x
			);
	}

	template <class T>
	float length(const Vector2<T> & v)
	{
		return sqrt(dot(v, v));
	}

	template <class T>
	float length(const Vector3<T> & v)
	{
		return sqrt(dot(v, v));
	}

	template <class T>
	float length(const Vector4<T> & v)
	{
		return sqrt(dot(v, v));
	}

	template <class T>
	Vector3<T> normalize(const Vector3<T> & v)
	{
		float l = length(v);
		return l == 0.0f ? 0.0f : v / l;
	}

	template <class T>
	Vector4<T> normalize(const Vector4<T> & v)
	{
		float l = length(v);
		return l == 0.0f ? 0.0f : v / l;
	}


	template <class T>
	Vector4<T> mul(const Vector4<T> & vec, const Matrix4<T> & mat)
	{
		return Vector4 <T>
			(
			vec[0] * mat[0][0] + vec[1] * mat[1][0] + vec[2] * mat[2][0] + vec[3] * mat[3][0],
			vec[0] * mat[0][1] + vec[1] * mat[1][1] + vec[2] * mat[2][1] + vec[3] * mat[3][1],
			vec[0] * mat[0][2] + vec[1] * mat[1][2] + vec[2] * mat[2][2] + vec[3] * mat[3][2],
			vec[0] * mat[0][3] + vec[1] * mat[1][3] + vec[2] * mat[2][3] + vec[3] * mat[3][3],
			);
	}

	template <class T>
	Matrix4<T> mul(const Matrix4<T> & mat0, const Matrix4<T> & mat1)
	{
		return Matrix4 <T>
			(
			mul(mat0[0], mat1),
			mul(mat0[1], mat1),
			mul(mat0[2], mat1),
			mul(mat0[3], mat1)
			);
	}

	template <class T>
	T det(const Matrix4<T> & mat)
	{
		const T _2233_3223 = mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3];
		const T _2133_3123 = mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3];
		const T _2132_3122 = mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2];
		const T _2033_3023 = mat[2][0] * mat[3][3] - mat[3][0] * mat[2][3];
		const T _2032_3022 = mat[2][0] * mat[3][2] - mat[3][0] * mat[2][2];
		const T _2031_3021 = mat[2][0] * mat[3][1] - mat[3][0] * mat[2][1];

		//const T M00_11 = _2233_3223;
		//const T M00_12 = _2133_3123;
		//const T M00_13 = _2132_3122;
		const T M00 = mat[1][1] * _2233_3223 - mat[1][2] * _2133_3123 + mat[1][3] * _2132_3122;

		//const T M01_10 = _2233_3223;
		//const T M01_12 = _2033_3023;
		//const T M01_13 = _2032_3022;
		const T M01 = mat[1][0] * _2233_3223 - mat[1][2] * _2033_3023 + mat[1][3] * _2032_3022;

		//const T M02_10 = _2133_3123;
		//const T M02_11 = _2033_3023;
		//const T M02_13 = _2031_3021;
		const T M02 = mat[1][0] * _2133_3123 - mat[1][1] * _2033_3023 + mat[1][3] * _2031_3021;

		//const T M03_10 = _2132_3122;
		//const T M03_11 = _2032_3022;
		//const T M03_12 = _2031_3021;
		const T M03 = mat[1][0] * _2132_3122 - mat[1][1] * _2032_3022 + mat[1][2] * _2031_3021;

		return mat[0][0] * M00 - mat[0][1] * M01 + mat[0][2] * M02 - mat[0][3] * M03;
	}

	class TOYGE_CORE_API Math
	{
	public:
		static void AxisAlignedBoxToMinMax(const XNA::AxisAlignedBox & aabb, float3 & min, float3 & max)
		{
			min.x = aabb.Center.x - aabb.Extents.x;
			min.y = aabb.Center.y - aabb.Extents.y;
			min.z = aabb.Center.z - aabb.Extents.z;

			max.x = aabb.Center.x + aabb.Extents.x;
			max.y = aabb.Center.y + aabb.Extents.y;
			max.z = aabb.Center.z + aabb.Extents.z;
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