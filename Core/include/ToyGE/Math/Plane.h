#pragma once
#ifndef PLANE_H
#define PLANE_H

namespace ToyGE
{
	template <typename T>
	class Plane_T
	{
	public:
		Vector<T, 4> value;

		Plane_T()
		{
		}

		Plane_T(const Vector<T, 4> & v)
			: value(v)
		{
		}

		Plane_T(const T & v0, const T & v1, const T & v2, const T & v3)
			: value({ v0, v1, v2, v3 })
		{
		}

		Plane_T(const Vector<T, 3> & normal, const T & w)
			: value({normal.x(), normal.y(), normal.z(), w})
		{
		}

		template <typename RT>
		Plane_T(const Vector<RT, 4> & v)
			: value(v)
		{
		}

		template <typename RT>
		Plane_T(const RT & v0, const RT & v1, const RT & v2, const RT & v3)
			: value({ v0, v1, v2, v3 })
		{
		}

		template <typename RT>
		Plane_T(const Vector<RT, 3> & normal, const RT & w)
			: value({ normal.x(), normal.y(), normal.z(), w })
		{
		}

		Vector<T, 3> normal() const
		{
			return value.recombine<3>({ 0, 1, 2 });
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

		Plane_T<T> & operator=(const Plane_T & rhs)
		{
			value = rhs.value;
			return *this;
		}

		template <typename RT>
		Plane_T<T> & operator=(const Plane_T<RT> & rhs)
		{
			value = rhs.value;
			return *this;
		}

		Plane_T<T> & operator+=(const Plane_T<T> & rhs)
		{
			value += rhs;
			return *this;
		}
		Plane_T<T> & operator-=(const Plane_T<T> & rhs)
		{
			value -= rhs.value;
			return *this;
		}
		Plane_T<T> & operator*=(const Plane_T<T> & rhs)
		{
			value *= rhs.value;
			return *this;
		}
		Plane_T<T> & operator/=(const Plane_T<T> & rhs)
		{
			value /= rhs.value;
			return *this;
		}

		Vector<bool, 4> operator<(const Plane_T<T> & rhs) const
		{
			return value < rhs.value;
		}
		Vector<bool, 4> operator<=(const Plane_T<T> & rhs) const
		{
			return value <= rhs.value;
		}
		Vector<bool, 4> operator>(const Plane_T<T> & rhs) const
		{
			return value > rhs.value;
		}
		Vector<bool, 4> operator>=(const Plane_T<T> & rhs) const
		{
			return value >= rhs.value;
		}
		Vector<bool, 4> operator==(const Plane_T<T> & rhs) const
		{
			return value == rhs.value;
		}
		Vector<bool, 4> operator!=(const Plane_T<T> & rhs) const
		{
			return value != rhs.value;
		}

		Plane_T<T> operator+() const
		{
			return *this;
		}
		Plane_T<T> operator-() const
		{
			return Plane_T<T>(-value);
		}

		void swap(const Plane_T<T> & rhs)
		{
			using std::swap;
			swap(value, rhs.value);
		}

		friend Plane_T<T> operator+(const Plane_T<T> & lhs, const Plane_T<T> & rhs)
		{
			Plane_T<T> r = lhs;
			return (r += rhs);
		}
		friend Plane_T<T> operator-(const Plane_T<T> & lhs, const Plane_T<T> & rhs)
		{
			Plane_T<T> r = lhs;
			return (r -= rhs);
		}
		friend Plane_T<T> operator*(const Plane_T<T> & lhs, const Plane_T<T> & rhs)
		{
			Plane_T<T> r = lhs;
			return (r *= rhs);
		}
		friend Plane_T<T> operator/(const Plane_T<T> & lhs, const Plane_T<T> & rhs)
		{
			Plane_T<T> r = lhs;
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

		const T & y() const
		{
			return (*this)[1];
		}
		T & y()
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

		const T & w() const
		{
			return (*this)[3];
		}
		T & w()
		{
			return (*this)[3];
		}
	};

	template <typename T>
	void swap(const Plane_T<T> & lhs, const Plane_T<T> & rhs)
	{
		lhs.swap(rhs);
	}
}

#endif
