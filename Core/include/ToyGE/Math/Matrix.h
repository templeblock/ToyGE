#pragma once
#ifndef MATRIX_H
#define MATRIX_H

#include "ToyGE\Math\Vector.h"

namespace ToyGE
{
	template <class T>
	class Matrix4
	{
	public:
		Vector4< Vector4<T> > _m;

		Matrix4(){};

		Matrix4(
			const T & m00, const T & m01, const T & m02, const T & m03,
			const T & m10, const T & m11, const T & m12, const T & m13,
			const T & m20, const T & m21, const T & m22, const T & m23,
			const T & m30, const T & m31, const T & m32, const T & m33)
			: _m(
			Vector4<T>(m00, m01, m02, m03),
			Vector4<T>(m10, m11, m12, m13),
			Vector4<T>(m20, m21, m22, m23),
			Vector4<T>(m30, m31, m32, m33))
		{
		}

		Matrix4(const Vector4<T> & v0, const Vector4<T> & v1, const Vector4<T> & v2, const Vector4<T> & v3)
			:_m(v0, v1, v2, v3){}

		Matrix4(const T & v) : _m( Vector4<T>(v) ){}

		Vector4<T> & operator[](size_t i)
		{
			return _m[i];
		}

		const Vector4<T> & operator[](size_t i) const
		{
			return _m[i];
		}

		Vector4<T> & operator()(size_t row, size_t col)
		{
			return (*this)[row][col];
		}

		const Vector4<T> & operator()(size_t row, size_t col) const
		{
			return (*this)[row][col];
		}

		bool operator==(const Matrix4 & rhs)
		{
			return _m == rhs._m;
		}

		Matrix4<T> & operator+=(const Matrix4<T> & rhs)
		{
			_m += rhs;
			return *this;
		}

		Matrix4<T> & operator-=(const Matrix4<T> & rhs)
		{
			_m -= rhs;
			return *this;
		}

		Matrix4<T> & operator*=(const Matrix4<T> & rhs)
		{
			_m += rhs;
			return *this;
		}

		Matrix4<T> & operator/=(const Matrix4<T> & rhs)
		{
			_m += rhs;
			return *this;
		}

	};

	template <class T>
	Matrix4<T> operator+(const Matrix4<T> & lhs, const Matrix4<T> & rhs)
	{
		Matrix4<T> ret(lhs);
		ret += rhs;
		return ret;
	}

	template <class T>
	Matrix4<T> operator-(const Matrix4<T> & lhs, const Matrix4<T> & rhs)
	{
		Matrix4<T> ret(lhs);
		ret -= rhs;
		return ret;
	}

	template <class T>
	Matrix4<T> operator*(const Matrix4<T> & lhs, const Matrix4<T> & rhs)
	{
		Matrix4<T> ret(lhs);
		ret *= rhs;
		return ret;
	}

	template <class T>
	Matrix4<T> operator/(const Matrix4<T> & lhs, const Matrix4<T> & rhs)
	{
		Matrix4<T> ret(lhs);
		ret /= rhs;
		return ret;
	}
}

#endif