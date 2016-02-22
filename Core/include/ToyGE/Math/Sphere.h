#pragma once
#ifndef SPHERE_H
#define SPHERE_H

namespace ToyGE
{
	template <typename T>
	class Sphere_T
	{
	public:
		Vector<T, 3> center;
		T radius;

		Sphere_T()
		{
		}

		Sphere_T(const Vector<T, 3> & center, const T & radius)
		{
			this->center = center;
			this->radius = radius;
		}

		bool operator==(const Sphere_T<T> & rhs) const
		{
			return all(center == rhs.center) && radius == rhs.radius;
		}
	};
}

#endif
