#ifndef TOPAZ_CORE_MATRIX_HPP
#define TOPAZ_CORE_MATRIX_HPP
#include <array>
#include <concepts>

namespace tz
{
	/**
	 * @ingroup tz_core_math
	 * @brief Represents the generic matrix.
	 * @internal
	 */
	template<typename T, int N>
	requires std::integral<T> || std::floating_point<T>
	struct matrix
	{
		/// Retrieve a matrix filled with zeroes.
		static constexpr matrix<T, N> zero()
		{
			return matrix<T, N>::filled(T{0});
		}

		/// Retrieve an identity matrix that represents no transform.
		static constexpr matrix<T, N> iden()
		{
			auto ret = matrix<T, N>::zero();
			for(std::size_t i = 0; i < N; i++)
			{
				ret.mat[i * N + i] = T{1};
			}
			return ret;
		}
	
		/// Retrieve a matrix filled with the given value.
		static constexpr matrix<T, N> filled(T t)
		{
			matrix<T, N> ret;
			std::fill(ret.mat.begin(), ret.mat.end(), t);
			return ret;
		}

		matrix<T, N>() = default;
		matrix(const matrix<T, N>& cpy) = default;
		matrix(matrix<T, N>&& move) = default;
		matrix& operator=(const matrix<T, N>& rhs) = default;
		matrix& operator=(matrix<T, N>&& rhs) = default;

		const T& operator[](std::size_t idx) const;
		T& operator[](std::size_t idx);

		const T& operator()(std::size_t x, std::size_t y) const;
		T& operator()(std::size_t x, std::size_t y);

		/// Create a matrix that causes the inverse transformation. Such that the product of this and the result are the identity matrix.
		matrix<T, N> inverse() const;
		/// Retrieve a copy of this matrix but with its rows and columns swapped.
		matrix<T, N> transpose() const;

		// matrix-scalar
		matrix<T, N>& operator+=(T scalar);
		matrix<T, N> operator+(T scalar) const{auto cpy = *this; return cpy += scalar;}
		matrix<T, N>& operator-=(T scalar);
		matrix<T, N> operator-(T scalar) const{auto cpy = *this; return cpy -= scalar;}
		matrix<T, N>& operator*=(T scalar);
		matrix<T, N> operator*(T scalar) const{auto cpy = *this; return cpy *= scalar;}
		matrix<T, N>& operator/=(T scalar);
		matrix<T, N> operator/(T scalar) const{auto cpy = *this; return cpy /= scalar;}

		// matrix-matrix
		matrix<T, N>& operator*=(const matrix<T, N>& rhs);
		matrix<T, N> operator*(const matrix<T, N>& rhs) const{auto cpy = *this; return cpy *= rhs;}

		bool operator==(const matrix<T, N>& rhs) const = default;

		private:
		std::array<T, N*N> mat;
	};

	using m2i = matrix<int, 2>;
	using m3i = matrix<int, 3>;
	using m4i = matrix<int, 4>;
	using m2u = matrix<unsigned int, 2>;
	using m3u = matrix<unsigned int, 3>;
	using m4u = matrix<unsigned int, 4>;
	using m2f = matrix<float, 2>;
	using m3f = matrix<float, 3>;
	using m4f = matrix<float, 4>;
	using m2d = matrix<double, 2>;
	using m3d = matrix<double, 3>;
	using m4d = matrix<double, 4>;
}

#endif // TOPAZ_CORE_MATRIX_HPP