#pragma once
#ifndef VECTOR_H
#define VECTOR_H

#include "ToyGE\Kernel\PreInclude.h"

namespace ToyGE
{
	template <typename T, int N>
	class Vector
	{
	public:
		T value[N];

		Vector()
		{
		}

		Vector(const T & v0, const T & v1)
			: Vector({ v0, v1 })
		{
		}
		Vector(const T & v0, const T & v1, const T & v2)
			: Vector({ v0, v1, v2 })
		{
		}
		Vector(const T & v0, const T & v1, const T & v2, const T & v3)
			: Vector({ v0, v1, v2, v3 })
		{
		}

		Vector(const T & v)
		{
			for (auto & i : value)
				i = v;
		}

		Vector(const std::initializer_list<T> & v)
		{
			auto itr = v.begin();
			for (int i = 0; i < N && itr != v.end(); ++i)
				value[i] = *(itr++);
		}

		template <typename RT>
		Vector(const RT & v)
		{
			for (auto & i : value)
				i = static_cast<RT>(v);
		}

		template <typename RT>
		Vector(const std::initializer_list<RT> & v)
		{
			auto itr = v.begin();
			for (int i = 0; i < N && itr != v.end(); ++i)
				value[i] = static_cast<RT>(*(itr++));
		}

		Vector(const Vector<T, N> & rhs)
		{
			*this = rhs;
		}

		template <typename RT>
		Vector(const Vector<RT, N> & rhs)
		{
			for (int i = 0; i < N; ++i)
				value[i] = static_cast<RT>(rhs.value[i]);
		}

		const T & operator[](int index) const
		{
			return value[index];
		}
		T & operator[](int index)
		{
			return value[index];
		}

		const T & operator()(int index) const
		{
			return value[index];
		}
		T & operator()(int index)
		{
			return value[index];
		}

		Vector<T, N> & operator=(const Vector<T, N> & rhs)
		{
			std::copy(std::begin(rhs.value), std::end(rhs.value), std::begin(value));
			return *this;
		}

		Vector<T, N> & operator+=(const Vector<T, N> & rhs)
		{
			for (int i = 0; i < N; ++i)
				value[i] += rhs.value[i];
			return *this;
		}
		Vector<T, N> & operator-=(const Vector<T, N> & rhs)
		{
			for (int i = 0; i < N; ++i)
				value[i] -= rhs.value[i];
			return *this;
		}
		Vector<T, N> & operator*=(const Vector<T, N> & rhs)
		{
			for (int i = 0; i < N; ++i)
				value[i] *= rhs.value[i];
			return *this;
		}
		Vector<T, N> & operator/=(const Vector<T, N> & rhs)
		{
			for (int i = 0; i < N; ++i)
				value[i] /= rhs.value[i];
			return *this;
		}

		Vector<bool, N> operator<(const Vector<T, N> & rhs) const
		{
			Vector<bool, N> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] < rhs.value[i];
			return r;
		}
		Vector<bool, N> operator<=(const Vector<T, N> & rhs) const
		{
			Vector<bool, N> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] <= rhs.value[i];
			return r;
		}
		Vector<bool, N> operator>(const Vector<T, N> & rhs) const
		{
			Vector<bool, N> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] > rhs.value[i];
			return r;
		}
		Vector<bool, N> operator>=(const Vector<T, N> & rhs) const
		{
			Vector<bool, N> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] >= rhs.value[i];
			return r;
		}
		Vector<bool, N> operator==(const Vector<T, N> & rhs) const
		{
			Vector<bool, N> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] == rhs.value[i];
			return r;
		}
		Vector<bool, N> operator!=(const Vector<T, N> & rhs) const
		{
			Vector<bool, N> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] != rhs.value[i];
			return r;
		}

		Vector<T, N> operator+() const
		{
			return *this;
		}
		Vector<T, N> operator-() const
		{
			return (*this) * -1;
		}

		void swap(const Vector<T, N> & rhs)
		{
			using std::swap;
			for (int i = 0; i < N; ++i)
				swap(value[i], rhs.value[i]);
		}

		friend Vector<T, N> operator+(const Vector<T, N> & lhs, const Vector<T, N> & rhs)
		{
			Vector<T, N> r = lhs;
			return (r += rhs);
		}
		friend Vector<T, N> operator-(const Vector<T, N> & lhs, const Vector<T, N> & rhs)
		{
			Vector<T, N> r = lhs;
			return (r -= rhs);
		}
		friend Vector<T, N> operator*(const Vector<T, N> & lhs, const Vector<T, N> & rhs)
		{
			Vector<T, N> r = lhs;
			return (r *= rhs);
		}
		friend Vector<T, N> operator/(const Vector<T, N> & lhs, const Vector<T, N> & rhs)
		{
			Vector<T, N> r = lhs;
			return (r /= rhs);
		}


		const T & x() const
		{
			return (*this)[0];
		}
		T & x()
		{
			return (*this)[0];
		}
		const T & r() const
		{
			return (*this)[0];
		}
		T & r()
		{
			return (*this)[0];
		}

		const T & y() const
		{
			return (*this)[1];
		}
		T & y()
		{
			return (*this)[1];
		}
		const T & g() const
		{
			return (*this)[1];
		}
		T & g()
		{
			return (*this)[1];
		}

		const T & z() const
		{
			return (*this)[2];
		}
		T & z()
		{
			return (*this)[2];
		}
		const T & b() const
		{
			return (*this)[2];
		}
		T & b()
		{
			return (*this)[2];
		}

		const T & w() const
		{
			return (*this)[3];
		}
		T & w()
		{
			return (*this)[3];
		}
		const T & a() const
		{
			return (*this)[3];
		}
		T & a()
		{
			return (*this)[3];
		}
	};

}

#endif