#pragma once
#ifndef QUATERNION_H
#define QUATERNION_H

namespace ToyGE
{
	template <typename T>
	class Quaternion_T : public Vector<T, 4>
	{
	public:
		using Vector<T, 4>::Vector;

		Quaternion_T()
		{

		}

		Quaternion_T(const Vector<T, 3> & v, const T & w)
			: Vector(v.x(), v.y(), v.z(), w)
		{
		}

		Vector<T, 3> v() const
		{
			return recombine<3>({ 0, 1, 2 });
		}
	};
}

#endif
