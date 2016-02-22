#pragma once
#ifndef MATRIX_H
#define MATRIX_H

namespace ToyGE
{
	template <typename T, int N, int M>
	class Matrix
	{
	public:
		Vector<Vector<T, M>, N> value;

		Matrix()
		{
		}

		Matrix(const T & v)
		{
			for (int i = 0; i < N; ++i)
				for (int j = 0; j < M; ++j)
					value[i][j] = v;
		}

		template <typename RT>
		Matrix(const RT & v)
		{
			for (int i = 0; i < N; ++i)
				for (int j = 0; j < M; ++j)
					value[i][j] = static_cast<T>(v);
		}

		Matrix(const std::initializer_list<T> & v)
		{
			auto itr = v.begin();
			for (int i = 0; i < N && itr != v.end(); ++i)
				for (int j = 0; j < M && itr != v.end(); ++j)
					value[i][j] = *(itr++);
		}

		Matrix(const std::initializer_list<Vector<T, M>> & v)
		{
			auto itr = v.begin();
			for (int i = 0; i < N && itr != v.end(); ++i)
					value[i] = *(itr++);
		}	

		template <typename RT>
		Matrix(const std::initializer_list<RT> & v)
		{
			auto itr = v.begin();
			for (int i = 0; i < N && itr != v.end(); ++i)
				for (int j = 0; j < M && itr != v.end(); ++j)
					value[i][j] = static_cast<T>(*(itr++));
		}

		template <typename RT>
		Matrix(const std::initializer_list<Vector<RT, M>> & v)
		{
			auto itr = v.begin();
			for (int i = 0; i < N && itr != v.end(); ++i)
				value[i] = *(itr++);
		}

		Matrix(const Matrix<T, N, M> & rhs)
		{
			*this = rhs;
		}

		template <typename RT, int K, int L>
		Matrix(const Matrix<RT, K, L> & rhs)
		{
			*this = rhs;
		}

		Matrix<T, N, M> & operator=(const Matrix<T, N, M> & rhs)
		{
			for (int i = 0; i < N; ++i)
				for (int j = 0; j < M; ++j)
					value[i][j] = (rhs[i][j]);
			return *this;
		}

		template <typename RT, int K, int L>
		Matrix<T, N, M> & operator=(const Matrix<RT, K, L> & rhs)
		{
			int nr = std::min<int>(N, K);
			int nc = std::min<int>(M, L);
			for (int i = 0; i < nr; ++i)
				for (int j = 0; j < nc; ++j)
					value[i][j] = static_cast<T>(rhs[i][j]);
			return *this;
		}

		const Vector<T, M> & row(int index) const
		{
			return value[index];
		}
		Vector<T, N> col(int index) const
		{
			Vector<T, N> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i][index];
			return r;
		}

		const Vector<T, M> & operator[](int index) const
		{
			return value[index];
		}
		Vector<T, M> & operator[](int index)
		{
			return value[index];
		}

		const T & operator()(int row, int col) const
		{
			return value[row][col];
		}
		T & operator()(int row, int col)
		{
			return value[row][col];
		}

		Matrix<T, N, M> & operator+=(const Matrix<T, N, M> & rhs)
		{
			for (int i = 0; i < N; ++i)
				value[i] += rhs.value[i];
			return *this;
		}
		Matrix<T, N, M> & operator-=(const Matrix<T, N, M> & rhs)
		{
			for (int i = 0; i < N; ++i)
				value[i] -= rhs.value[i];
			return *this;
		}
		Matrix<T, N, M> & operator*=(const Matrix<T, N, M> & rhs)
		{
			(*this) = mul(*this, rhs);
			return *this;
		}
		/*Matrix<T, N, M> & operator/=(const Matrix<T, N, M> & rhs)
		{
			for (int i = 0; i < N; ++i)
				value[i] /= rhs.value[i];
			return *this;
		}*/

		Matrix<bool, N, M> operator<(const Matrix<T, N, M> & rhs) const
		{
			Matrix<bool, N, M> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] < rhs.value[i];
			return r;
		}
		Matrix<bool, N, M> operator<=(const Matrix<T, N, M> & rhs) const
		{
			Matrix<bool, N, M> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] <= rhs.value[i];
			return r;
		}
		Matrix<bool, N, M> operator>(const Matrix<T, N, M> & rhs) const
		{
			Matrix<bool, N, M> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] > rhs.value[i];
			return r;
		}
		Matrix<bool, N, M> operator>=(const Matrix<T, N, M> & rhs) const
		{
			Matrix<bool, N, M> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] >= rhs.value[i];
			return r;
		}
		Matrix<bool, N, M> operator==(const Matrix<T, N, M> & rhs) const
		{
			Matrix<bool, N, M> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] == rhs.value[i];
			return r;
		}
		Matrix<bool, N, M> operator!=(const Matrix<T, N, M> & rhs) const
		{
			Matrix<bool, N, M> r;
			for (int i = 0; i < N; ++i)
				r[i] = value[i] != rhs.value[i];
			return r;
		}

		Matrix<T, N, M> operator+() const
		{
			return *this;
		}
		Matrix<T, N, M> operator-() const
		{
			return (*this) * -1;
		}

		void swap(const Matrix<T, N, M> & rhs)
		{
			using std::swap;
			for (int i = 0; i < N; ++i)
				swap(value[i], rhs.value[i]);
		}

		friend Matrix<T, N, M> operator+(const Matrix<T, N, M> & lhs, const Matrix<T, N, M> & rhs)
		{
			Matrix<T, N, M> r = lhs;
			return (r += rhs);
		}
		friend Matrix<T, N, M> operator-(const Matrix<T, N, M> & lhs, const Matrix<T, N, M> & rhs)
		{
			Matrix<T, N, M> r = lhs;
			return (r -= rhs);
		}
		friend Matrix<T, N, M> operator*(const Matrix<T, N, M> & lhs, const Matrix<T, N, M> & rhs)
		{
			Matrix<T, N, M> r = lhs;
			return (r *= rhs);
		}
		friend Matrix<T, N, M> operator/(const Matrix<T, N, M> & lhs, const Matrix<T, N, M> & rhs)
		{
			Matrix<T, N, M> r = lhs;
			return (r /= rhs);
		}
	};

	template <typename T, int N, int M>
	void swap(const Matrix<T, N, M> & lhs, const Matrix<T, N, M> & rhs)
	{
		lhs.swap(rhs);
	}
}

#endif