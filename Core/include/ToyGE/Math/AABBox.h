#pragma once
#ifndef AABBOX_H
#define AABBOX_H

#include "ToyGE\Math\Vector.h"

namespace ToyGE
{
	template <typename T>
	class AABBox_T
	{
	public:
		Vector<T, 3> min;
		Vector<T, 3> max;

		AABBox_T()
		{
		}

		AABBox_T(const Vector<T, 3> & min, const Vector<T, 3> & max)
		{
			this->min = min;
			this->max = max;
		}

		bool operator==(const AABBox_T<T> & rhs) const
		{
			return all(min == rhs.min) && all( max == rhs.max);
		}

		AABBox_T<T> & operator&=(const AABBox_T<T> & rhs)
		{
			min = max_vec<T>(min, rhs.min);
			max = min_vec<T>(max, rhs.max);
		}
		AABBox_T<T> & operator|=(const AABBox_T<T> & rhs)
		{
			min = min_vec<T>(min, rhs.min);
			max = max_vec<T>(max, rhs.max);
		}

		AABBox_T<T> & Add(const Vector<T, 3> & v)
		{
			min = min_vec<T>(min, v);
			max = max_vec<T>(max, v);
		}

		Vector<T, 3> Center() const
		{
			return (min + max) / 2;
		}

		Vector<T, 3> Extents() const
		{
			return (max - min) / 2;
		}

		T Width() const 
		{
			return this->max.x() - this->max.x();
		}

		T Height() const 
		{
			return this->max.y() - this->max.y();
		}

		T Depth() const 
		{
			return this->max.z() - this->max.z();
		}

		Vector<T, 3> LeftBottomNear() const
		{
			return this->min;
		}

		Vector<T, 3> LeftTopNear() const
		{
			return Vector<T, 3>(this->min.x(), this->max.y(), this->min.z());
		}

		Vector<T, 3> RightBottomNear() const
		{
			return Vector<T, 3>(this->max.x(), this->min.y(), this->min.z());
		}

		Vector<T, 3> RightTopNear() const 
		{
			return Vector<T, 3>(this->max.x(), this->max.y(), this->min.z());
		}

		Vector<T, 3> LeftBottomFar() const 
		{
			return Vector<T, 3>(this->min.x(), this->min.y(), this->max.z());
		}

		Vector<T, 3> LeftTopFar() const 
		{
			return Vector<T, 3>(this->min.x(), this->max.y(), this->max.z());
		}

		Vector<T, 3> RightBottomFar() const 
		{
			return Vector<T, 3>(this->max.x(), this->min.y(), this->max.z());
		}

		Vector<T, 3> RightTopFar() const 
		{
			return this->max;
		}

		Vector<T, 3> Corner(int index) const
		{
			return Vector<T, 3>(
				(index & 1UL) ? this->max.x() : this->min.x(),
				(index & 2UL) ? this->max.y() : this->min.y(),
				(index & 4UL) ? this->max.z() : this->min.z());
		}

		bool Contains(const AABBox_T<T> & rhs) const
		{
			return all(min <= rhs.min) && all(max >= rhs.max);
		}
	};
}

#endif
