#include "tz/core/matrix.hpp"
#include <limits>
#include <utility>
#include <cmath>

namespace tz
{
#define MATRIX_TEMPLATE_IMPL \
    template<typename T, int N> \
    requires std::integral<T> || std::floating_point<T>

	MATRIX_TEMPLATE_IMPL
	const T& matrix<T, N>::operator[](std::size_t idx) const
	{
		return this->mat[idx];
	}

	MATRIX_TEMPLATE_IMPL
	T& matrix<T, N>::operator[](std::size_t idx)
	{
		return this->mat[idx];
	}

	MATRIX_TEMPLATE_IMPL
	const T& matrix<T, N>::operator()(std::size_t x, std::size_t y) const
	{
		return this->mat[x + (y * N)];
	}

	MATRIX_TEMPLATE_IMPL
	T& matrix<T, N>::operator()(std::size_t x, std::size_t y)
	{
		return this->mat[x + (y * N)];
	}

	MATRIX_TEMPLATE_IMPL
	matrix<T, N> matrix<T, N>::inverse() const
	{
		matrix<T, N> inv = matrix<T, N>::iden();  // Start with identity matrix as the inverse
		matrix<T, N> copy = *this; // Copy of the current matrix

		for(std::size_t i = 0; i < N; i++)
		{
			// Find pivot row and swap
			std::size_t pivot = i;
			for(std::size_t j = i + 1; j < N; j++)
			{
				// abs madness.
				// this is just: if(abs(mat[j * N + i] > abs(mat[pivot * N + i])))
				bool cond;
				if constexpr(std::is_floating_point_v<T>)
				{
					cond = std::fabs(copy.mat[j * N + i]) > std::fabs(copy.mat[pivot * N + i]);
				}
				else if constexpr(std::is_signed_v<T>)
				{
					cond = std::labs(copy.mat[j * N + i]) > std::labs(copy.mat[pivot * N + i]);
				}
				else
				{
					cond = std::abs(static_cast<std::int64_t>(copy.mat[j * N + i])) > std::abs(static_cast<std::int64_t>(copy.mat[pivot * N + i]));
				}
				if(cond)
				{
					pivot = j;
				}
			}

			if(copy.mat[pivot * N + i] == T{0})
			{
				// matrix is singular so there is no inverse.
				return matrix<T, N>::filled(T{0});
			}

			// Swap rows in both the copy and the inverse matrix
			if(pivot != i)
			{
				for(std::size_t k = 0; k < N; k++)
				{
					std::swap(copy.mat[i * N + k], copy.mat[pivot * N + k]);
					std::swap(inv.mat[i * N + k], inv.mat[pivot * N + k]);
				}
			}

			// Normalize the pivot row
			T pivot_val = copy.mat[i * N + i];
			for(std::size_t k = 0; k < N; k++)
			{
				copy.mat[i * N + k] /= pivot_val;
				inv.mat[i * N + k] /= pivot_val;
			}

			// Eliminate other rows
			for(std::size_t j = 0; j < N; j++)
			{
				if(j != i)
				{
					T factor = copy.mat[j * N + i];
					for(std::size_t k = 0; k < N; k++)
					{
						copy.mat[j * N + k] -= factor * copy.mat[i * N + k];
						inv.mat[j * N + k] -= factor * inv.mat[i * N + k];
					}
				}
			}
		}
		return inv;
	}

	MATRIX_TEMPLATE_IMPL
	matrix<T, N> matrix<T, N>::transpose() const
	{
		auto cpy = *this;
		for(std::size_t i = 0; i < N; i++)
		{
			for(std::size_t j = 0; j < N; j++)
			{
				cpy.mat[j * N + i] = this->mat[i * N + j];
			}
		}
		return cpy;
	}

	MATRIX_TEMPLATE_IMPL
	matrix<T, N>& matrix<T, N>::operator+=(T scalar)
	{
		for(std::size_t i = 0; i < N*N; i++)
		{
			this->mat[i] += scalar;
		}
		return *this;
	}

	MATRIX_TEMPLATE_IMPL
	matrix<T, N>& matrix<T, N>::operator-=(T scalar)
	{
		for(std::size_t i = 0; i < N*N; i++)
		{
			this->mat[i] -= scalar;
		}
		return *this;
	}

	MATRIX_TEMPLATE_IMPL
	matrix<T, N>& matrix<T, N>::operator*=(T scalar)
	{
		for(std::size_t i = 0; i < N*N; i++)
		{
			this->mat[i] *= scalar;
		}
		return *this;
	}

	MATRIX_TEMPLATE_IMPL
	matrix<T, N>& matrix<T, N>::operator/=(T scalar)
	{
		for(std::size_t i = 0; i < N*N; i++)
		{
			this->mat[i] /= scalar;
		}
		return *this;
	}

	MATRIX_TEMPLATE_IMPL
	matrix<T, N>& matrix<T, N>::operator*=(const matrix<T, N>& rhs)
	{
		matrix<T, N> result = matrix<T, N>::zero(); // Start with zero matrix for the result

		for(std::size_t i = 0; i < N; ++i) 
		{
			for(std::size_t j = 0; j < N; ++j) 
			{
				for(std::size_t k = 0; k < N; ++k) 
				{
					result.mat[i * N + j] += this->mat[i * N + k] * rhs.mat[k * N + j];
				}
			}
		}

		*this = result;
		return *this;
	}

    template struct matrix<int, 2>;
	template struct matrix<int, 3>;
	template struct matrix<int, 4>;
    template struct matrix<unsigned int, 2>;
	template struct matrix<unsigned int, 3>;
	template struct matrix<unsigned int, 4>;
    template struct matrix<float, 2>;
	template struct matrix<float, 3>;
	template struct matrix<float, 4>;
    template struct matrix<double, 2>;
	template struct matrix<double, 3>;
	template struct matrix<double, 4>;
}