#ifndef TOPAZ_CORE_VECTOR_HPP
#define TOPAZ_CORE_VECTOR_HPP
#include "core/types.hpp"
#include "core/algorithms/static.hpp"
#include <array>
#include <tuple>

namespace tz
{
	/**
	 * @ingroup tz_core_vecmat
	 * Represents a mathematical vector of S elements.
	 * Typically, elements would be floats or doubles, but in this implementation they are templated on T.
	 */
	template<tz::Number T, std::size_t S>
	class Vector
	{
	public:
		/**
		 * Construct a vector directly using a variadic parameter pack value.
		 * Static Precondition: sizeof...(Ts) == S, otherwise the program is ill-formed.
		 */
		template<tz::Number... Ts>
		constexpr Vector(Ts&&... ts) :
		vec({ std::forward<Ts>(ts)... }) {}
		/**
		 * Construct a vector using an existing array of appropriate size.
		 */
		constexpr Vector(std::array<T, S> data);
		/**
		 * Default-constructed vectors have indeterminate values.
		 */
		Vector() = default;
		/**
		 * Retrieve the element value at the given index.
		 * Precondition: idx < S. Otherwise, this will assert and invoke UB.
		 * @return The value at the given index.
		 */
		const T& operator[](std::size_t idx) const;
		/**
		 * Retrieve the element value at the given index.
		 * Precondition: idx < S. Otherwise, this will assert and invoke UB.
		 * @return The value at the given index.
		 */
		T& operator[](std::size_t idx);
		/**
		 * Add each element of the given vector to the corresponding element of the current vector.
		 * @param rhs Vector whose elements need be added to the current vector.
		 * @return The modified original vector.
		 */
		Vector<T, S>& operator+=(const Vector<T, S>& rhs);
		/**
		 * Create a copy of the current vector, add each element of the given vector to the corresponding element of the copied vector and return the result.
		 * @param rhs Vector whose elemens need be added to the current vector.
		 * @return A modified copy of the current vector. The initial vector is unchanged.
		 */
		Vector<T, S> operator+(const Vector<T, S>& rhs) const;
		/**
		 * Subtract each element of the given vector from the corresponding element of the current vector.
		 * @param rhs Vector whose elements need be subtracted from the current vector.
		 * @return The modified original vector.
		 */
		Vector<T, S>& operator-=(const Vector<T, S>& rhs);
		/**
		 * Create a copy of the current vector, subtract each element of the given vector from the corresponding element of the copied vector and return the result.
		 * @param rhs Vector whose elemens need be subtracted from the current vector.
		 * @return A modified copy of the current vector. The initial vector is unchanged.
		 */
		Vector<T, S> operator-(const Vector<T, S>& rhs) const;
		/**
		 * Multiply each element of the current vector by the given value.
		 * @param scalar Value to multiply by each element of the current vector.
		 * @return The modified original vector.
		 */
		Vector<T, S>& operator*=(T scalar);
		/**
		 * Create a copy of the current vector, multiply each element of the copied vector by the given value, and return the result.
		 * @param scalar Value to multiply by each element of the current vector.
		 * @return A modified copy of the current vector. The initial vector is unchanged.
		 */
		Vector<T, S> operator*(T scalar) const;
		/**
		 * Divide each element of the current vector by the given value.
		 * @param scalar Value to divide by each element of the current vector.
		 * @return The modified original vector.
		 */
		Vector<T, S>& operator/=(T scalar);
		/**
		 * Create a copy of the current vector, divide each element of the copied vector by the given value, and return the result.
		 * @param scalar Value to divide by each element of the current vector.
		 * @return A modified copy of the current vector. The initial vector is unchanged.
		 */
		Vector<T, S> operator/(T scalar) const;

		/**
		 * Equate the current vector with the given vector.
		 * @param rhs Given vector to compare with the current vector.
		 * @return True if each element of the given vector are equal to the corresponding element of the current vector.
		 */
		bool operator==(const Vector<T, S>& rhs) const;
		/**
		 * Retrieve a pointer to the vector data. This points to the first element of an array of size S.
		 * @return Pointer to the first element of the vector array data.
		 */
		const T* data() const;
		/**
		 * Retrieve a pointer to the vector data. This points to the first element of an array of size S.
		 * @return Pointer to the first element of the vector array data.
		 */
		T* data();
		/**
		 * Compute a dot-product (otherwise known as the scalar-product) of the current vector against another given vector.
		 * @param rhs The other given vector.
		 * @return Scalar value representing the dot-product.
		 */
		T dot(const Vector<T, S>& rhs) const;
		/**
		 * Retrieve the magnitude of the current vector.
		 * @return Magnitude of the vector.
		 */
		T length() const;
		/**
		 * Normalise the vector. This is equivalent to dividing the vector by its length().
		 * Note: This modifies the current vector. To retrieve a modified copy and leave the original unchanged, see Vector<T, S>::normalised().
		 */
		void normalise();
		/**
		 * Create a copy of the vector, normalise it and return the result.
		 * @return A normalised copy of the current vector.
		 */
		Vector<T, S> normalised() const;
	private:
		std::array<T, S> vec;
	};

	/**
	 * @ingroup tz_core_vecmat
	 * @{
	 */
	/// A vector of two floats.
	using Vec2 = Vector<float, 2>;
	/// A vector of three floats.
	using Vec3 = Vector<float, 3>;
	/// A vector of four floats.
	using Vec4 = Vector<float, 4>;

	/// A vector of two ints.
	using Vec2i = Vector<int, 2>;
	/// A vector of three ints.
	using Vec3i = Vector<int, 3>;
	/// A vector of four ints.
	using Vec4i = Vector<int, 4>;
	
	/// A vector of two unsigned ints.
	using Vec2ui = Vector<unsigned int, 2>;
	/// A vector of three unsigned ints.
	using Vec3ui = Vector<unsigned int, 3>;
	/// A vector of four unsigned ints.
	using Vec4ui = Vector<unsigned int, 4>;

	template<typename T = float>
	Vector<T, 3> cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs);

	/**
	 * @}
	 */
}

#include "core/vector.inl"
#endif // TOPAZ_CORE_VECTOR_HPP
