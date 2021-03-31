#include "core/debug/assert.hpp"
#if TOPAZ_DEBUG
#include <cstdio>
#endif

namespace tz
{
	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C>::Matrix(std::array<std::array<T, R>, C> data): mat(data){}

	template<typename T, std::size_t R, std::size_t C>
	const typename Matrix<T, R, C>::Row& Matrix<T, R, C>::operator[](std::size_t row_idx) const
	{
		topaz_assert(row_idx < R, "tz::Matrix<T, ", R, ", ", C, ">::operator[", row_idx, "]: Index out of range!");
		return this->mat[row_idx];
	}

	template<typename T, std::size_t R, std::size_t C>
	typename Matrix<T, R, C>::Row& Matrix<T, R, C>::operator[](std::size_t row_idx)
	{
		topaz_assert(row_idx < R, "tz::Matrix<T, ", R, ", ", C, ">::operator[", row_idx, "]: Index out of range!");
		return this->mat[row_idx];
	}

	template<typename T, std::size_t R, std::size_t C>
	const T& Matrix<T, R, C>::operator()(std::size_t row, std::size_t column) const
	{
		topaz_assert(row < R, "tz::Matrix<T, ", R, ", ", C, ">::operator(", row, ", ", column, "): Row index out of range!");
		topaz_assert(column < R, "tz::Matrix<T, ", C, ", ", C, ">::operator(", row, ", ", column, "): Column index out of range!");
		return (*this)[column][row];
	}

	template<typename T, std::size_t R, std::size_t C>
	T& Matrix<T, R, C>::operator()(std::size_t row, std::size_t column)
	{
		topaz_assert(row < R, "tz::Matrix<T, ", R, ", ", C, ">::operator(", row, ", ", column, "): Row index out of range!");
		topaz_assert(column < C, "tz::Matrix<T, ", R, ", ", C, ">::operator(", row, ", ", column, "): Column index out of range!");
		return (*this)[column][row];
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C>& Matrix<T, R, C>::operator+=(T scalar)
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				(*this)(i, j) += scalar;
			}
		}
		return *this;
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C>& Matrix<T, R, C>::operator+=(const Matrix<T, R, C>& matrix)
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				(*this)(i, j) += matrix(i, j);
			}
		}
		return *this;
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C> Matrix<T, R, C>::operator+(T scalar) const
	{
		Matrix<T, R, C> copy = *this;
		copy += scalar;
		return std::move(copy);
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C> Matrix<T, R, C>::operator+(const Matrix<T, R, C>& matrix) const
	{
		Matrix<T, R, C> copy = *this;
		copy += matrix;
		return std::move(copy);
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C>& Matrix<T, R, C>::operator-=(T scalar)
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				(*this)(i, j) -= scalar;
			}
		}
		return *this;
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C>& Matrix<T, R, C>::operator-=(const Matrix<T, R, C>& matrix)
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				(*this)(i, j) -= matrix(i, j);
			}
		}
		return *this;
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C> Matrix<T, R, C>::operator-(T scalar) const
	{
		Matrix<T, R, C> copy = *this;
		copy -= scalar;
		return std::move(copy);
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C> Matrix<T, R, C>::operator-(const Matrix<T, R, C>& matrix) const
	{
		Matrix<T, R, C> copy = *this;
		copy -= matrix;
		return std::move(copy);
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C>& Matrix<T, R, C>::operator*=(T scalar)
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				(*this)(i, j) *= scalar;
			}
		}
		return *this;
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C>& Matrix<T, R, C>::operator*=(const Matrix<T, R, C>& matrix)
	{
		Matrix<T, R, C> m = *this;
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				T res_ele = T();
				for(std::size_t k = 0; k < C; k++)
				{
					res_ele += (m(i, k) * matrix(k, j));
				}
				(*this)(i, j) = res_ele;
			}
		}
		return *this;
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C> Matrix<T, R, C>::operator*(T scalar) const
	{
		Matrix<T, R, C> copy = *this;
		copy *= scalar;
		return copy;
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C> Matrix<T, R, C>::operator*(const Matrix<T, R, C>& matrix) const
	{
		Matrix<T, R, C> copy = *this;
		copy *= matrix;
		return copy;
	}

	template<typename T, std::size_t R, std::size_t C>
	Vector<T, R> Matrix<T, R, C>::operator*(const Vector<T, C>& vec) const
	{
		Vector<T, R> ret;
		for(std::size_t i = 0; i < R; i++)
		{
			ret[i] = tz::Vec4{(*this)[i]}.dot(vec);
		}
		return ret;
	}

	template<typename T, std::size_t R, std::size_t C>
	bool Matrix<T, R, C>::operator==(T scalar) const
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				if((*this)(i, j) != scalar)
					return false;
			}
		}
		return true;
	}

	template<typename T, std::size_t R, std::size_t C>
	bool Matrix<T, R, C>::operator==(const Matrix<T, R, C>& matrix) const
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				if((*this)(i, j) != matrix(i, j))
					return false;
			}
		}
		return true;
	}

	template<typename T, std::size_t R, std::size_t C>
	Matrix<T, R, C> Matrix<T, R, C>::inverse() const
	{
		// Create copy of the current matrix to work with.
		Matrix<T, R, C> mat = Matrix<T, R, C>::identity();
		// TODO: Replace with Jacobi's Method.
		
		// Column-major
		auto at = [](Matrix<T, R, C>& m, std::size_t idx)->T&
		{
			// 5 ==> (1, 1)
			// row_id = 5 / 4 == 1
			// column_id = (1*4)
			std::size_t row_id = idx / C;
			std::size_t column_id = idx - (row_id*C);
			return m(row_id, column_id);
		};

		auto cat = [](const Matrix<T, R, C>& m, std::size_t idx)->const T&
		{
			// 5 ==> (1, 1)
			// row_id = 5 / 4 == 1
			// column_id = (1*4)
			std::size_t row_id = idx / C;
			std::size_t column_id = idx - (row_id*C);
			return m(row_id, column_id);
		};

		at(mat, 0) = cat(*this, 5) * cat(*this, 10) * cat(*this, 15) -
				cat(*this, 5) * cat(*this, 11) * cat(*this, 14) -
				cat(*this, 9) * cat(*this, 6)  * cat(*this, 15) +
				cat(*this, 9) * cat(*this, 7)  * cat(*this, 14) + 
				cat(*this, 13) * cat(*this, 6) * cat(*this, 11) - 
				cat(*this, 13) * cat(*this, 7) * cat(*this, 10);

		at(mat, 4) = -cat(*this, 4)  * cat(*this, 10) * cat(*this, 15) + 
				cat(*this, 4)  * cat(*this, 11) * cat(*this, 14) + 
				cat(*this, 8)  * cat(*this, 6)  * cat(*this, 15) - 
				cat(*this, 8)  * cat(*this, 7)  * cat(*this, 14) - 
				cat(*this, 12) * cat(*this, 6)  * cat(*this, 11) + 
				cat(*this, 12) * cat(*this, 7)  * cat(*this, 10);

		at(mat, 8) = cat(*this, 4)  * cat(*this, 9) * cat(*this, 15) - 
				cat(*this, 4)  * cat(*this, 11) * cat(*this, 13) - 
				cat(*this, 8)  * cat(*this, 5) * cat(*this, 15) + 
				cat(*this, 8)  * cat(*this, 7) * cat(*this, 13) + 
				cat(*this, 12) * cat(*this, 5) * cat(*this, 11) - 
				cat(*this, 12) * cat(*this, 7) * cat(*this, 9);

		at(mat, 12) = -cat(*this, 4)  * cat(*this, 9) * cat(*this, 14) + 
				cat(*this, 4)  * cat(*this, 10) * cat(*this, 13) +
				cat(*this, 8)  * cat(*this, 5) * cat(*this, 14) - 
				cat(*this, 8)  * cat(*this, 6) * cat(*this, 13) - 
				cat(*this, 12) * cat(*this, 5) * cat(*this, 10) + 
				cat(*this, 12) * cat(*this, 6) * cat(*this, 9);

		at(mat, 1) = -cat(*this, 1)  * cat(*this, 10) * cat(*this, 15) + 
				cat(*this, 1)  * cat(*this, 11) * cat(*this, 14) + 
				cat(*this, 9)  * cat(*this, 2) * cat(*this, 15) - 
				cat(*this, 9)  * cat(*this, 3) * cat(*this, 14) - 
				cat(*this, 13) * cat(*this, 2) * cat(*this, 11) + 
				cat(*this, 13) * cat(*this, 3) * cat(*this, 10);

		at(mat, 5) = cat(*this, 0)  * cat(*this, 10) * cat(*this, 15) - 
				cat(*this, 0)  * cat(*this, 11) * cat(*this, 14) - 
				cat(*this, 8)  * cat(*this, 2) * cat(*this, 15) + 
				cat(*this, 8)  * cat(*this, 3) * cat(*this, 14) + 
				cat(*this, 12) * cat(*this, 2) * cat(*this, 11) - 
				cat(*this, 12) * cat(*this, 3) * cat(*this, 10);

		at(mat, 9) = -cat(*this, 0)  * cat(*this, 9) * cat(*this, 15) + 
				cat(*this, 0)  * cat(*this, 11) * cat(*this, 13) + 
				cat(*this, 8)  * cat(*this, 1) * cat(*this, 15) - 
				cat(*this, 8)  * cat(*this, 3) * cat(*this, 13) - 
				cat(*this, 12) * cat(*this, 1) * cat(*this, 11) + 
				cat(*this, 12) * cat(*this, 3) * cat(*this, 9);

		at(mat, 13) = cat(*this, 0)  * cat(*this, 9) * cat(*this, 14) - 
				cat(*this, 0)  * cat(*this, 10) * cat(*this, 13) - 
				cat(*this, 8)  * cat(*this, 1) * cat(*this, 14) + 
				cat(*this, 8)  * cat(*this, 2) * cat(*this, 13) + 
				cat(*this, 12) * cat(*this, 1) * cat(*this, 10) - 
				cat(*this, 12) * cat(*this, 2) * cat(*this, 9);

		at(mat, 2) = cat(*this, 1)  * cat(*this, 6) * cat(*this, 15) - 
				cat(*this, 1)  * cat(*this, 7) * cat(*this, 14) - 
				cat(*this, 5)  * cat(*this, 2) * cat(*this, 15) + 
				cat(*this, 5)  * cat(*this, 3) * cat(*this, 14) + 
				cat(*this, 13) * cat(*this, 2) * cat(*this, 7) - 
				cat(*this, 13) * cat(*this, 3) * cat(*this, 6);

		at(mat, 6) = -cat(*this, 0)  * cat(*this, 6) * cat(*this, 15) + 
				cat(*this, 0)  * cat(*this, 7) * cat(*this, 14) + 
				cat(*this, 4)  * cat(*this, 2) * cat(*this, 15) - 
				cat(*this, 4)  * cat(*this, 3) * cat(*this, 14) - 
				cat(*this, 12) * cat(*this, 2) * cat(*this, 7) + 
				cat(*this, 12) * cat(*this, 3) * cat(*this, 6);

		at(mat, 10) = cat(*this, 0)  * cat(*this, 5) * cat(*this, 15) - 
				cat(*this, 0)  * cat(*this, 7) * cat(*this, 13) - 
				cat(*this, 4)  * cat(*this, 1) * cat(*this, 15) + 
				cat(*this, 4)  * cat(*this, 3) * cat(*this, 13) + 
				cat(*this, 12) * cat(*this, 1) * cat(*this, 7) - 
				cat(*this, 12) * cat(*this, 3) * cat(*this, 5);

		at(mat, 14) = -cat(*this, 0)  * cat(*this, 5) * cat(*this, 14) + 
				cat(*this, 0)  * cat(*this, 6) * cat(*this, 13) + 
				cat(*this, 4)  * cat(*this, 1) * cat(*this, 14) - 
				cat(*this, 4)  * cat(*this, 2) * cat(*this, 13) - 
				cat(*this, 12) * cat(*this, 1) * cat(*this, 6) + 
				cat(*this, 12) * cat(*this, 2) * cat(*this, 5);

		at(mat, 3) = -cat(*this, 1) * cat(*this, 6) * cat(*this, 11) + 
				cat(*this, 1) * cat(*this, 7) * cat(*this, 10) + 
				cat(*this, 5) * cat(*this, 2) * cat(*this, 11) - 
				cat(*this, 5) * cat(*this, 3) * cat(*this, 10) - 
				cat(*this, 9) * cat(*this, 2) * cat(*this, 7) + 
				cat(*this, 9) * cat(*this, 3) * cat(*this, 6);

		at(mat, 7) = cat(*this, 0) * cat(*this, 6) * cat(*this, 11) - 
				cat(*this, 0) * cat(*this, 7) * cat(*this, 10) - 
				cat(*this, 4) * cat(*this, 2) * cat(*this, 11) + 
				cat(*this, 4) * cat(*this, 3) * cat(*this, 10) + 
				cat(*this, 8) * cat(*this, 2) * cat(*this, 7) - 
				cat(*this, 8) * cat(*this, 3) * cat(*this, 6);

		at(mat, 11) = -cat(*this, 0) * cat(*this, 5) * cat(*this, 11) + 
				cat(*this, 0) * cat(*this, 7) * cat(*this, 9) + 
				cat(*this, 4) * cat(*this, 1) * cat(*this, 11) - 
				cat(*this, 4) * cat(*this, 3) * cat(*this, 9) - 
				cat(*this, 8) * cat(*this, 1) * cat(*this, 7) + 
				cat(*this, 8) * cat(*this, 3) * cat(*this, 5);

		at(mat, 15) = cat(*this, 0) * cat(*this, 5) * cat(*this, 10) - 
				cat(*this, 0) * cat(*this, 6) * cat(*this, 9) - 
				cat(*this, 4) * cat(*this, 1) * cat(*this, 10) + 
				cat(*this, 4) * cat(*this, 2) * cat(*this, 9) + 
				cat(*this, 8) * cat(*this, 1) * cat(*this, 6) - 
				cat(*this, 8) * cat(*this, 2) * cat(*this, 5);

		float determinant = cat(*this, 0) * cat(mat, 0) + cat(*this, 1) * cat(mat, 4) + cat(*this, 2) * cat(mat, 8) + cat(*this, 3) * cat(mat, 12);
		topaz_assert(determinant != 0, "tz::geo::Matrix<T, ", R, ", ", C, ">::inverse(): Cannot get inverse because determinant is zero.");
		determinant = 1.0f / determinant;
		for(std::size_t i = 0; i < 16; i++)
			at(mat, i) = cat(mat, i) * determinant;
		return mat;
	}

	template<typename T, std::size_t R, std::size_t C>
	//template<std::size_t X, std::size_t Y, typename std::enable_if_t<X == Y>>
	Matrix<T, R, C> Matrix<T, R, C>::transpose() const
	{
		Matrix<T, R, C> m = *this;
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				if(i != j)
				{
					std::swap(m(i, j), m(j, i));
				}
			}
		}
		return m;
	}

	#if TOPAZ_DEBUG
	template<typename T, std::size_t R, std::size_t C>
	void Matrix<T, R, C>::debug_print() const
	{

		for(std::size_t i = 0; i < R; i++)
		{
			std::printf("|");
			for(std::size_t j = 0; j < C; j++)
			{
				if(j > 0 && j <= (C - 1))
				{
					std::printf("   ");
				}
				std::printf("%g", (*this)(i, j));
			}
			std::printf("|\n"); 
		}
		std::printf("\n");
	}
	#endif
}