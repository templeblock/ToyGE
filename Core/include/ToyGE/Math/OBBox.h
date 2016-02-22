#pragma once
#ifndef OBBOX_H
#define OBBOX_H

#include "ToyGE\Math\AABBox.h"

namespace ToyGE
{
	template <typename T>
	class OBBox_T
	{
	public:
		AABBox_T<T> aabb;
		Quaternion_T<T> orientation;

		OBBox_T()
		{
		}

		OBBox_T(const AABBox_T<T> & aabb, const Quaternion_T<T> & orientation)
		{
			this->aabb = aabb;
			this->orientation = orientation;
		}

		bool operator==(const OBBox_T<T> & rhs) const
		{
			return aabb = rhs.aabb && all(orientation == rhs.orientation);
		}

		Vector<T, 3> Axis(int index) const
		{
			Vector<T, 3> v(0, 0, 0);
			v[index] = 1;
			return transform_quat(v, orientation);
		}

		Vector<T, 3> Corner(int index) const
		{
			Vector<T, 3> const & center = this->aabb.Center();
			Vector<T, 3> const & extent = this->aabb.Extents();
			Vector<T, 3> const extent_x = abs(extent.x() * this->Axis(0));
			Vector<T, 3> const extent_y = abs(extent.y() * this->Axis(1));
			Vector<T, 3> const extent_z = abs(extent.z() * this->Axis(2));

			return center
				+ ((index & 1UL) ? +extent_x : -extent_x)
				+ ((index & 2UL) ? +extent_y : -extent_y)
				+ ((index & 4UL) ? +extent_z : -extent_z);
		}

		Vector<T, 3> Center() const
		{
			return aabb.Center();
		}

		Vector<T, 3> Extents() const
		{
			return aabb.Extents();
		}
	};
}

#endif
