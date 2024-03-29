#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include "tz/dbgui/dbgui.hpp"
#if TOPAZ_DEBUG
#include <cstdio>
#endif

namespace tz
{
	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C>::matrix(std::array<std::array<T, R>, C> data): mat(data){}

	template<tz::number T, std::size_t R, std::size_t C>
	const typename matrix<T, R, C>::Row& matrix<T, R, C>::operator[](std::size_t row_idx) const
	{
		//tz::assert(row_idx < R, "tz::matrix<T, %zu, %zu>::operator[%zu]: Index out of range!", R, C, row_idx);
		return this->mat[row_idx];
	}

	template<tz::number T, std::size_t R, std::size_t C>
	typename matrix<T, R, C>::Row& matrix<T, R, C>::operator[](std::size_t row_idx)
	{
		//tz::assert(row_idx < R, "tz::matrix<T, %zu, %zu>::operator[%zu]: Index out of range!", R, C, row_idx);
		return this->mat[row_idx];
	}

	template<tz::number T, std::size_t R, std::size_t C>
	const T& matrix<T, R, C>::operator()(std::size_t row, std::size_t column) const
	{
		//tz::assert(row < R, "tz::matrix<T, %zu, %zu>::operator(%zu, %zu): Row index out of range!", R, C, row, column);
		//tz::assert(column < R, "tz::matrix<T, %zu, %zu>::operator(%zu, %zu): Column index out of range!", R, C, row, column);
		return (*this)[column][row];
	}

	template<tz::number T, std::size_t R, std::size_t C>
	T& matrix<T, R, C>::operator()(std::size_t row, std::size_t column)
	{
		//tz::assert(row < R, "tz::matrix<T, %zu, %zu>::operator(%zu, %zu): Row index out of range!", R, C, row, column);
		//tz::assert(column < R, "tz::matrix<T, %zu, %zu>::operator(%zu, %zu): Column index out of range!", R, C, row, column);
		return (*this)[column][row];
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C>& matrix<T, R, C>::operator+=(T scalar)
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

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C>& matrix<T, R, C>::operator+=(const matrix<T, R, C>& matrix)
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

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C> matrix<T, R, C>::operator+(T scalar) const
	{
		matrix<T, R, C> copy = *this;
		copy += scalar;
		return std::move(copy);
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C> matrix<T, R, C>::operator+(const matrix<T, R, C>& mat) const
	{
		matrix<T, R, C> copy = *this;
		copy += mat;
		return std::move(copy);
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C>& matrix<T, R, C>::operator-=(T scalar)
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

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C>& matrix<T, R, C>::operator-=(const matrix<T, R, C>& matrix)
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

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C> matrix<T, R, C>::operator-(T scalar) const
	{
		matrix<T, R, C> copy = *this;
		copy -= scalar;
		return std::move(copy);
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C> matrix<T, R, C>::operator-(const matrix<T, R, C>& mat) const
	{
		matrix<T, R, C> copy = *this;
		copy -= mat;
		return std::move(copy);
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C>& matrix<T, R, C>::operator*=(T scalar)
	{
		TZ_PROFZONE("Matrix - Scalar Multiply", 0xFF0000AA);
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				(*this)(i, j) *= scalar;
			}
		}
		return *this;
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C>& matrix<T, R, C>::operator*=(const matrix<T, R, C>& mat)
	{
		TZ_PROFZONE("Matrix - Matrix Multiply", 0xFF0000AA);
		matrix<T, R, C> m = *this;
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				T res_ele = T();
				for(std::size_t k = 0; k < C; k++)
				{
					res_ele += (m.internal_get(i, k) * mat.internal_get(k, j));
				}
				this->internal_get(i, j) = res_ele;
			}
		}
		return *this;
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C> matrix<T, R, C>::operator*(T scalar) const
	{
		matrix<T, R, C> copy = *this;
		copy *= scalar;
		return copy;
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C> matrix<T, R, C>::operator*(const matrix<T, R, C>& mat) const
	{
		matrix<T, R, C> copy = *this;
		copy *= mat;
		return copy;
	}

	template<tz::number T, std::size_t R, std::size_t C>
	tz::vector<T, R> matrix<T, R, C>::operator*(const tz::vector<T, C>& vec) const
	{
		TZ_PROFZONE("Matrix - Vector Multiply", 0xFF0000AA);
		tz::vector<T, R> ret;
		for(std::size_t i = 0; i < R; i++)
		{
			ret[i] = tz::vec4{(*this)[i]}.dot(vec);
		}
		return ret;
	}

	template<tz::number T, std::size_t R, std::size_t C>
	bool matrix<T, R, C>::operator==(T scalar) const
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				if(this->internal_get(i, j) != scalar)
					return false;
			}
		}
		return true;
	}

	template<tz::number T, std::size_t R, std::size_t C>
	bool matrix<T, R, C>::operator==(const matrix<T, R, C>& mat) const
	{
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				if(this->internal_get(i, j) != mat.internal_get(i, j))
					return false;
			}
		}
		return true;
	}

	template<tz::number T, std::size_t R, std::size_t C>
	matrix<T, R, C> matrix<T, R, C>::inverse() const
	{
		TZ_PROFZONE("Matrix - Inverse", 0xFF0000AA);
		// Create copy of the current matrix to work with.
		matrix<T, R, C> mat = matrix<T, R, C>::identity();
		// TODO: Replace with Jacobi's Method.
		
		// Column-major
		auto at = [](matrix<T, R, C>& m, std::size_t idx)->T&
		{
			// 5 ==> (1, 1)
			// row_id = 5 / 4 == 1
			// column_id = (1*4)
			std::size_t row_id = idx / C;
			std::size_t column_id = idx - (row_id*C);
			return m.internal_get(row_id, column_id);
		};

		auto cat = [](const matrix<T, R, C>& m, std::size_t idx)->const T&
		{
			// 5 ==> (1, 1)
			// row_id = 5 / 4 == 1
			// column_id = (1*4)
			std::size_t row_id = idx / C;
			std::size_t column_id = idx - (row_id*C);
			return m.internal_get(row_id, column_id);
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
		tz::assert(determinant != 0, "tz::geo::matrix<T, %zu, %zu>::inverse(): Cannot get inverse because determinant is zero.", R, C);
		determinant = 1.0f / determinant;
		for(std::size_t i = 0; i < 16; i++)
			at(mat, i) = cat(mat, i) * determinant;
		return mat;
	}

	template<tz::number T, std::size_t R, std::size_t C>
	//template<std::size_t X, std::size_t Y, typename std::enable_if_t<X == Y>>
	matrix<T, R, C> matrix<T, R, C>::transpose() const
	{
		TZ_PROFZONE("Matrix - Transpose", 0xFF0000AA);
		matrix<T, R, C> m = *this;
		for(std::size_t i = 0; i < R; i++)
		{
			for(std::size_t j = 0; j < C; j++)
			{
				if(i != j)
				{
					std::swap(m.internal_get(i, j), m.internal_get(j, i));
				}
			}
		}
		return m;
	}

	#if TZ_DEBUG
	template<tz::number T, std::size_t R, std::size_t C>
	void matrix<T, R, C>::debug_print() const
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

	template<tz::number T, std::size_t R, std::size_t C>
	void matrix<T, R, C>::dbgui()
	{
		constexpr float matrix_cell_width = 35.0f;
		for(std::size_t row = 0; row < R; row++)
		{
			for(std::size_t col = 0; col < C; col++)
			{
				std::string address = std::to_string((unsigned long long)(void**)this);
				std::string label = "##m" + address + std::to_string(row) + std::to_string(col);
				ImGui::SetNextItemWidth(matrix_cell_width);
				ImGui::InputFloat(label.c_str(), &this->operator()(row, col));
				ImGui::SameLine();
			}
			ImGui::NewLine();
		}
	}

	template<tz::number T, std::size_t R, std::size_t C>
	const T& matrix<T, R, C>::internal_get(std::size_t row, std::size_t column) const
	{
		return this->mat[column][row];
	}

	template<tz::number T, std::size_t R, std::size_t C>
	T& matrix<T, R, C>::internal_get(std::size_t row, std::size_t column)
	{
		return this->mat[column][row];
	}

	template<tz::number T>
	T determinant(const matrix<T, 4, 4>& m)
	{

		// Calculate 2x2 determinants
		T det2_01 = m[0][0] * m[1][1] - m[0][1] * m[1][0];
		T det2_02 = m[0][0] * m[1][2] - m[0][2] * m[1][0];
		T det2_03 = m[0][0] * m[1][3] - m[0][3] * m[1][0];
		T det2_12 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
		T det2_13 = m[0][1] * m[1][3] - m[0][3] * m[1][1];
		T det2_23 = m[0][2] * m[1][3] - m[0][3] * m[1][2];

		// Calculate 3x3 determinants
		T det3_201 = m[2][0] * det2_01 - m[2][1] * det2_02 + m[2][2] * det2_12;
		T det3_202 = m[2][0] * det2_01 - m[2][1] * det2_03 + m[2][3] * det2_13;
		T det3_203 = m[2][0] * det2_02 - m[2][2] * det2_03 + m[2][3] * det2_23;
		T det3_301 = m[3][0] * det2_01 - m[3][1] * det2_02 + m[3][2] * det2_12;
		T det3_302 = m[3][0] * det2_01 - m[3][1] * det2_03 + m[3][3] * det2_13;
		T det3_303 = m[3][0] * det2_02 - m[3][2] * det2_03 + m[3][3] * det2_23;

		// Calculate the final determinant
		return m[0][0] * det3_303 - m[0][1] * det3_302 + m[0][2] * det3_301 - m[0][3] * det3_203;
	}
}
