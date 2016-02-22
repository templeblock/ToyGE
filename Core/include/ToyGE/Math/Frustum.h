#pragma once
#ifndef FRUSTUM_H
#define FRUSTUM_H

namespace ToyGE
{
	template <typename T>
	class Frustum_T
	{
	public:
		std::array<Plane_T<T>, 6> planes;
		std::array<Vector<T, 3>, 8> corners;

		Frustum_T()
		{
		}

		Frustum_T(const std::array<Plane_T<T>, 6> & planes, std::array<Vector<T, 3>, 8> corners)
		{
			std::copy(planes.begin(), planes.end(), this->planes.begin());
			std::copy(corners.begin(), corners.end(), this->corners.begin());
		}

		bool operator==(const Frustum_T<T> & rhs) const
		{
			for (int i = 0; i < 6; ++i)
				if (!all(planes[i] == rhs.planes[i]))
					return false;
			for (int i = 0; i < 8; ++i)
				if (!all(corners[i] == rhs.corners[i]))
					return false;
			return true;
		}
	};
}

#endif