#ifndef TOPAZ_CORE_MATRIX_HPP
#define TOPAZ_CORE_MATRIX_HPP
#include "tz/core/data/vector.hpp"
#include "tz/core/types.hpp"
#include <array>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * Represents a row-major matrix with R rows and C columns.
	 * The value type of a matrix is typically a float or double, but for this implementation it is templated on T.
	 */
	template<tz::number T, std::size_t R, std::size_t C>
	class matrix
	{
	public:
		/**
		 * Default-intialised matrices have indeterminate values.
		 * To initialise the values, assign them or simply assign to matrix<T, R, C>::identity().
		 */
		matrix() = default;
		/**
		 * Initialise a matrix from a multi-dimensional array.
		 * @param data An array of array of values to construct the row-major matrix.
		 */
		matrix(std::array<std::array<T, R>, C> data);
		/**
		 * Retrieve the identity matrix.
		 * 
		 * High values (1) are constructed via T{1} and low values (0) are constructed via T{0}.
		 * @return The identity matrix for the given dimensions.
		 */
		static constexpr matrix<T, R, C> identity()
		{
			matrix<T, R, C> mat{{}};
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
		/// Type alias for an array large enough to hold a single *column* of the matrix.
		using Row = std::array<T, R>;
		/// Type alias for an array large enough to hold a single *row* of the matrix.
		using Column = std::array<T, C>;
		/**
		 * Retrieve the i'th row of the matrix.
		 * Precondition: row_idx < R. Otherwise, this will assert and invoke UB.
		 * @param row_idx The index of the row to retrieve. For example, row_idx 0 will retrieve the top row.
		 * @return Array of values representing the given row.
		 */
		const Row& operator[](std::size_t row_idx) const;
		/**
		 * Retrieve the i'th row of the matrix.
		 * Precondition: row_idx < R. Otherwise, this will assert and invoke UB.
		 * @param row_idx The index of the row to retrieve. For example, row_idx 0 will retrieve the top row.
		 * @return Array of values representing the given row.
		 */
		Row& operator[](std::size_t row_idx);
		/**
		 * Retrieve the element at the given row and column of the matrix.
		 * Precondition: row < R && column < C. Otherwise, this will assert and invoke UB.
		 * @param row The index of the row to retrieve. For example, row 0 will retrieve an element in the top-most row.
		 * @param column The index of the column. For example, column 0 will retrieve an element in the left-most row.
		 * @return The value at the given coordinate formed by [row, column]. For example, mymatrix(0, 0) will retrieve the very first element of the matrix.
		 */
		const T& operator()(std::size_t row, std::size_t column) const;
		/**
		 * Retrieve the element at the given row and column of the matrix.
		 * Precondition: row < R && column < C. Otherwise, this will assert and invoke UB.
		 * @param row The index of the row to retrieve. For example, row 0 will retrieve an element in the top-most row.
		 * @param column The index of the column. For example, column 0 will retrieve an element in the left-most row.
		 * @return The value at the given coordinate formed by [row, column]. For example, mymatrix(0, 0) will retrieve the very first element of the matrix.
		 */
		T& operator()(std::size_t row, std::size_t column);
		/**
		 * Add the scalar value to each element of the current matrix.
		 * @param scalar Value to add to each element.
		 * @return Reference to the current matrix.
		 */
		matrix<T, R, C>& operator+=(T scalar);
		/**
		 * Add the given matrix to the current matrix. This is done by adding the current value in the same row and column.
		 * @param matrix matrix to add to the current matrix.
		 * @return Reference to the current matrix.
		 */
		matrix<T, R, C>& operator+=(const matrix<T, R, C>& matrix);
		/**
		 * Add the scalar value to each element of the current matrix.
		 * @param scalar Value to add to each element.
		 * @return A modified copy of the original matrix. The original matrix is unchanged.
		 */
		matrix<T, R, C> operator+(T scalar) const;
		/**
		 * Add the given matrix to the current matrix. This is done by adding the current value in the same row and column.
		 * @param matrix matrix to add to the current matrix.
		 * @return A modified copy of the original matrix. The original matrix is unchanged.
		 */
		matrix<T, R, C> operator+(const matrix<T, R, C>& matrix) const;
		/**
		 * Subtract the scalar value from each element of the current matrix.
		 * @param scalar Value to subtract from each element.
		 * @return Reference to the current matrix.
		 */
		matrix<T, R, C>& operator-=(T scalar);
		/**
		 * Subtract the given matrix from the current matrix. This is done by subtracting the current value in the same row and column.
		 * @param matrix matrix to subtract from the current matrix.
		 * @return Reference to the current matrix.
		 */
		matrix<T, R, C>& operator-=(const matrix<T, R, C>& matrix);
		/**
		 * Subtract the scalar value from each element of the current matrix.
		 * @param scalar Value to subtract from each element.
		 * @return A modified copy of the original matrix. The original matrix is unchanged.
		 */
		matrix<T, R, C> operator-(T scalar) const;
		/**
		 * Subtract the given matrix from the current matrix. This is done by subtracting the current value in the same row and column.
		 * @param matrix matrix to subtract from the current matrix.
		 * @return A modified copy of the original matrix. The original matrix is unchanged.
		 */
		matrix<T, R, C> operator-(const matrix<T, R, C>& matrix) const;
		/**
		 * Multiply the scalar value with each element of the current matrix.
		 * @param scalar Value to multiply with each element.
		 * @return Reference to the current matrix.
		 */
		matrix<T, R, C>& operator*=(T scalar);
		/**
		 * Multiply the given matrix with the current matrix. This is done naively and is unsuitable for very large values of R or C.
		 * @param matrix matrix to multiply with the current matrix.
		 * @return Reference to the current matrix.
		 */
		matrix<T, R, C>& operator*=(const matrix<T, R, C>& matrix);
		/**
		 * Multiply the scalar value with each element of the current matrix.
		 * @param scalar Value to multiply with each element.
		 * @return A modified copy of the original matrix. The original matrix is unchanged.
		 */
		matrix<T, R, C> operator*(T scalar) const;
		/**
		 * Multiply the given matrix with the current matrix. This is done naively and is unsuitable for very large values of R or C.
		 * @param matrix matrix to multiply with the current matrix.
		 * @return A modified copy of the original matrix. The original matrix is unchanged.
		 */
		matrix<T, R, C> operator*(const matrix<T, R, C>& matrix) const;
		/**
		 * Multiply the given vector (column-matrix) with the current matrix and return the resultant vector (row-matrix).
		 * @param vec Column matrix to multiply with the current matrix.
		 * @return A row-matrix representing the resultant multiplication.
		 */
		tz::vector<T, R> operator*(const tz::vector<T, C>& vec) const;
		/**
		 * Equate the given scalar with each value of the matrix.
		 * `mymatrix == T{5}` shall return if-and-only-if each element of mymatrix is equal to 'T{5}'. If no such comparison exists, then the program is ill-formed.
		 * @param scalar Value to equate with each value of the current matrix.
		 * @return True if each value of the current matrix is equal to scalar. Otherwise, false.
		 */
		bool operator==(T scalar) const;
		/**
		 * Equate the given matrix with the current. Matrices are equal if-and-only-if each element at a given row and column of the current matrix is equal to the corresponding value of the given matrix.
		 * @param matrix matrix to equate with the current matrix.
		 * @return True if each value in the given matrix are equal to that of the current matrix. Otherwise, false.
		 */
		bool operator==(const matrix<T, R, C>& matrix) const;
		/**
		 * Retrieves a new matrix such that the resultant matrix could be multiplied by the original matrix to result in the identity matrix (See matrix<T, R, C>::identity()).
		 * 
		 * If the current matrix is orthogonal, it is an optimisation to prefer the use of matrix<T, R, C>::transpose() instead.
		 * @return Inverse of the original matrix. The original matrix is unchanged.
		 */
		matrix<T, R, C> inverse() const;
		//template<std::size_t X = R, std::size_t Y = C, typename std::enable_if_t<std::conditional_t<X == Y>>>
		/**
		 * Create a copy of the current matrix, and transpose the copy. Transposing the matrix is to flip the matrix values over its diagonal.
		 * Note: If `mymatrix` is orthogonal, then `mymatrix.transpose() == mymatrix.inverse()`.
		 * @return Transpose of the original matrix. The original matrix is unchanged.
		 */
		matrix<T, R, C> transpose() const;
		#if TZ_DEBUG
		/**
		 * Pretty-print each value of the given matrix. If T is not printable, then the program is ill-formed.
		 * Note: This method is compiled-out in release. If this method is invoked in a non-debug context, then the program is ill-formed.
		 */
		void debug_print() const;
		#endif
		void dbgui();
	private:
		const T& internal_get(std::size_t row, std::size_t column) const;
		T& internal_get(std::size_t row, std::size_t column);
		std::array<Row, C> mat;
	};

	/**
	 * @ingroup tz_core
	 * @{
	 */

	/// A 4x4 row-major matrix of floats.
	using mat4 = matrix<float, 4, 4>;
	/// A 3x3 row-major matrix of floats.
	using mat3 = matrix<float, 3, 3>;
	/// A 2x2 row-major matrix of floats.
	using mat2 = matrix<float, 2, 2>;

	/**
	 * @}
	 */
}

#include "tz/core/matrix.inl"
#endif // TOPAZ_CORE_MATRIX_HPP
