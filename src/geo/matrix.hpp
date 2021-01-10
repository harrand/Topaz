#ifndef TOPAZ_GEO_MATRIX_HPP
#define TOPAZ_GEO_MATRIX_HPP
#include "geo/vector.hpp"
#include <array>

namespace tz
{
	/**
	 * \addtogroup tz_geo Topaz Geometry Library (tz::geo)
	 * A collection of geometric data structures and mathematical types, such as vectors and matrices.
	 * @{
	 */

	template<typename T, std::size_t R, std::size_t C>
	class Matrix
	{
	public:
		Matrix() = default;
		Matrix(std::array<std::array<T, R>, C> data);

		static constexpr Matrix<T, R, C> identity()
		{
			Matrix<T, R, C> mat{{}};
			for(std::size_t i = 0; i < R; i++)
			{
				for(std::size_t j = 0; j < C; j++)
				{
					if(i == j)
						mat(i, j) = T{1};
					else
						mat(i, j) = T{0};
				}
			}
			return mat;
		}

		using Row = std::array<T, R>;
		using Column = std::array<T, C>;

		const Row& operator[](std::size_t row_idx) const;
		Row& operator[](std::size_t row_idx);
		const T& operator()(std::size_t row, std::size_t column) const;
		T& operator()(std::size_t row, std::size_t column);

		Matrix<T, R, C>& operator+=(T scalar);
		Matrix<T, R, C>& operator+=(const Matrix<T, R, C>& matrix);
		Matrix<T, R, C> operator+(T scalar) const;
		Matrix<T, R, C> operator+(const Matrix<T, R, C>& matrix) const;

		Matrix<T, R, C>& operator-=(T scalar);
		Matrix<T, R, C>& operator-=(const Matrix<T, R, C>& matrix);
		Matrix<T, R, C> operator-(T scalar) const;
		Matrix<T, R, C> operator-(const Matrix<T, R, C>& matrix) const;

		Matrix<T, R, C>& operator*=(T scalar);
		Matrix<T, R, C>& operator*=(const Matrix<T, R, C>& matrix);
		Matrix<T, R, C> operator*(T scalar) const;
		Matrix<T, R, C> operator*(const Matrix<T, R, C>& matrix) const;

		Vector<T, R> operator*(const Vector<T, C>& vec) const;

		bool operator==(T scalar) const;
		bool operator==(const Matrix<T, R, C>& matrix) const;

		Matrix<T, R, C> inverse() const;
		//template<std::size_t X = R, std::size_t Y = C, typename std::enable_if_t<std::conditional_t<X == Y>>>
		Matrix<T, R, C> transpose() const;

		#if TOPAZ_DEBUG
		void debug_print() const;
		#endif
	private:
		// 16-byte alignment to make it as easy as possible to auto-vectorise.
		std::array<Row, C> mat;
	};

	using Mat4 = Matrix<float, 4, 4>;
	using Mat3 = Matrix<float, 3, 3>;

	/**
	 * @}
	 */
}

#include "geo/matrix.inl"
#endif