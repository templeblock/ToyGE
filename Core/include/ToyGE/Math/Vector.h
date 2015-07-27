#pragma once
#ifndef VECTOR_H
#define VECTOR_H

namespace ToyGE
{
	enum VECTOR_VALUE
	{
		VEC_X = 0,
		VEC_Y = 1,
		VEC_Z = 2,
		VEC_W = 3
	};

	template <class T>
	class Vector2
	{
		friend Vector2<T> operator*(const Vector2<T> & lhs, const Vector2<T> & rhs)
		{
			Vector2<T> ret = lhs;
			ret *= rhs;
			return ret;
		}

		friend Vector2<T> operator-(const Vector2<T> & lhs, const Vector2<T> & rhs)
		{
			Vector2<T> ret = lhs;
			ret -= rhs;
			return ret;
		}

		friend Vector2<T> operator+(const Vector2<T> & lhs, const Vector2<T> & rhs)
		{
			Vector2<T> ret = lhs;
			ret += rhs;
			return ret;
		}

		friend Vector2<T> operator/(const Vector2<T> & lhs, const Vector2<T> & rhs)
		{
			Vector2<T> ret = lhs;
			ret /= rhs;
			return ret;
		}

	public:
		T x, y;

		Vector2(){}
		Vector2(const T & _x, const T & _y) : x(_x), y(_y) {}
		Vector2(const T & v) : x(v), y(v) {}

		Vector2<T> v(VECTOR_VALUE v0, VECTOR_VALUE v1)
		{
			return Vector2<T>(reinterpret_cast<T*>(this)[v0], reinterpret_cast<T*>(this)[v1]);
		}

		T & operator[](size_t i)
		{
			return reinterpret_cast<T*>(this)[i];
		}

		const T & operator[](size_t i) const
		{
			return reinterpret_cast<const T*>(this)[i];
		}

		Vector2<T> & operator+=(const Vector2<T> & rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		Vector2<T> & operator-=(const Vector2<T> & rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		Vector2<T> & operator*=(const Vector2<T> & rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}

		Vector2<T> & operator/=(const Vector2<T> & rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			return *this;
		}

		bool operator==(const Vector2<T> & rhs)
		{
			return x == rhs.x  && y == rhs.y;
		}

		bool operator<(const Vector2<T> & rhs)
		{
			return x < rhs.x && y < rhs.y;
		}

		bool operator<=(const Vector2<T> & rhs)
		{
			return x <= rhs.x && y <= rhs.y;
		}

		bool operator>(const Vector2<T> & rhs)
		{
			return x > rhs.x && y > rhs.y;
		}

		bool operator>=(const Vector2<T> & rhs)
		{
			return x >= rhs.x && y >= rhs.y;
		}
	};

	template <class T>
	class Vector3 : public Vector2 < T >
	{
		friend Vector3<T> operator*(const Vector3<T> & lhs, const Vector3<T> & rhs)
		{
			Vector3<T> ret = lhs;
			ret *= rhs;
			return ret;
		}

	public:
		T z;

		Vector3(){}
		Vector3(const T & _x, const T & _y, const T & _z) : Vector2<T>(_x, _y), z(_z) {}
		Vector3(const T & v) : Vector2<T>(v), z(v) {}

		using Vector2<T>::v;

		Vector3<T> v(VECTOR_VALUE v0, VECTOR_VALUE v1, VECTOR_VALUE v2)
		{
			return Vector3<T>(reinterpret_cast<T*>(this)[v0], reinterpret_cast<T*>(this)[v1], reinterpret_cast<T*>(this)[v2]);
		}

		Vector3<T> & operator+=(const Vector3<T> & rhs)
		{
			Vector2<T>::operator+=(rhs);
			z += rhs.z;
			return *this;
		}

		Vector3<T> & operator-=(const Vector3<T> & rhs)
		{
			Vector2<T>::operator-=(rhs);
			z -= rhs.z;
			return *this;
		}

		Vector3<T> & operator*=(const Vector3<T> & rhs)
		{
			Vector2<T>::operator*=(rhs);
			z *= rhs.z;
			return *this;
		}

		Vector3<T> & operator/=(const Vector3<T> & rhs)
		{
			Vector2<T>::operator/=(rhs);
			z /= rhs.z;
			return *this;
		}

		bool operator==(const Vector3<T> & rhs)
		{
			return Vector2<T>::operator==(rhs) && z == rhs.z;
		}

		bool operator<(const Vector3<T> & rhs)
		{
			return Vector2<T>::operator<(rhs) && z < rhs.z;
		}

		bool operator<=(const Vector3<T> & rhs)
		{
			return Vector2<T>::operator<=(rhs) && z <= rhs.z;
		}

		bool operator>(const Vector3<T> & rhs)
		{
			return Vector2<T>::operator>(rhs) && z > rhs.z;
		}

		bool operator>=(const Vector3<T> & rhs)
		{
			return Vector2<T>::operator>=(rhs) && z >= rhs.z;
		}

	};

	template <class T>
	class Vector4 : public Vector3 < T >
	{
		friend Vector4<T> operator*(const Vector4<T> & lhs, const Vector4<T> & rhs)
		{
			Vector4<T> ret = lhs;
			ret *= rhs;
			return ret;
		}

	public:
		T w;

		Vector4(){}
		Vector4(const T & _x, const T & _y, const T & _z, const T & _w) : Vector3<T>(_x, _y, _z), w(_w) {}
		Vector4(const T & v) : Vector3<T>(v), w(v){}

		using Vector3<T>::v;

		Vector4<T> v(VECTOR_VALUE v0, VECTOR_VALUE v1, VECTOR_VALUE v2, VECTOR_VALUE v3)
		{
			return Vector4<T>(reinterpret_cast<T*>(this)[v0], reinterpret_cast<T*>(this)[v1], reinterpret_cast<T*>(this)[v2], reinterpret_cast<T*>(this)[v3]);
		}

		Vector4<T> & operator+=(const Vector4<T> & rhs)
		{
			Vector3<T>::operator+=(rhs);
			w += rhs.w;
			return *this;
		}

		Vector4<T> & operator-=(const Vector4<T> & rhs)
		{
			Vector3<T>::operator-=(rhs);
			w -= rhs.w;
			return *this;
		}


		Vector4<T> & operator*=(const Vector4<T> & rhs)
		{
			Vector3<T>::operator*=(rhs);
			w *= rhs.w;
			return *this;
		}

		Vector4<T> & operator/=(const Vector4<T> & rhs)
		{
			Vector3<T>::operator/=(rhs);
			w /= rhs.w;
			return *this;
		}

		bool operator==(const Vector4<T> & rhs)
		{
			return Vector3<T>::operator==(rhs) && w == rhs.w;
		}
	};

	template <class T>
	Vector2<T> operator+(const Vector2<T> & lhs, const Vector2<T> & rhs)
	{
		Vector2<T> ret(lhs);
		ret += rhs;
		return ret;
	}

	template <class T>
	Vector2<T> operator-(const Vector2<T> & lhs, const Vector2<T> & rhs)
	{
		Vector2<T> ret(lhs);
		ret -= rhs;
		return ret;
	}

	template <class T>
	Vector2<T> operator*(const Vector2<T> & lhs, const Vector2<T> & rhs)
	{
		Vector2<T> ret(lhs);
		ret *= rhs;
		return ret;
	}

	template <class T>
	Vector2<T> operator/(const Vector2<T> & lhs, const Vector2<T> & rhs)
	{
		Vector2<T> ret(lhs);
		ret /= rhs;
		return ret;
	}


	template <class T>
	Vector3<T> operator+(const Vector3<T> & lhs, const Vector3<T> & rhs)
	{
		Vector3<T> ret(lhs);
		ret += rhs;
		return ret;
	}

	template <class T>
	Vector3<T> operator-(const Vector3<T> & lhs, const Vector3<T> & rhs)
	{
		Vector3<T> ret(lhs);
		ret -= rhs;
		return ret;
	}
	template <class T>
	Vector3<T> operator*(const Vector3<T> & lhs, const Vector3<T> & rhs)
	{
		Vector3<T> ret(lhs);
		ret *= rhs;
		return ret;
	}
	template <class T>
	Vector3<T> operator/(const Vector3<T> & lhs, const Vector3<T> & rhs)
	{
		Vector3<T> ret(lhs);
		ret /= rhs;
		return ret;
	}
	template <class T>
	Vector3<T> operator/(const Vector3<T> & lhs, const T & rhs)
	{
		return lhs / Vector3<T>(rhs);
	}


	template <class T>
	Vector4<T> operator+(const Vector4<T> & lhs, const Vector4<T> & rhs)
	{
		Vector4<T> ret(lhs);
		ret += rhs;
		return ret;
	}

	template <class T>
	Vector4<T> operator-(const Vector4<T> & lhs, const Vector4<T> & rhs)
	{
		Vector4<T> ret(lhs);
		ret -= rhs;
		return ret;
	}

	template <class T>
	Vector4<T> operator*(const Vector4<T> & lhs, const Vector4<T> & rhs)
	{
		Vector4<T> ret(lhs);
		ret *= rhs;
		return ret;
	}

	template <class T>
	Vector4<T> operator/(const Vector4<T> & lhs, const Vector4<T> & rhs)
	{
		Vector4<T> ret(lhs);
		ret += rhs;
		return ret;
	}
}

#endif