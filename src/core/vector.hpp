#ifndef TOPAZ_CORE_VECTOR_HPP
#define TOPAZ_CORE_VECTOR_HPP
#include "core/types.hpp"
#include "core/algorithms/static.hpp"
#include <array>
#include <tuple>
#include <span>

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
		 * Create a new vector, where every element is T{N}.
		 */
		template<int N>
		static constexpr Vector<T, S> filled_with();
		/**
		 * Create a new vector, where every element is T{0}.
		 *
		 * The resultant vector is equivalent to `Vector<T, S>::filled<0>()`
		 */
		static constexpr Vector<T, S> zero();
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
		std::span<const T> data() const;
		/**
		 * Retrieve a pointer to the vector data. This points to the first element of an array of size S.
		 * @return Pointer to the first element of the vector array data.
		 */
		std::span<T> data();
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

		/**
		 * Perform vector swizzling.
		 *
		 * Creates a new vector of `sizeof...(indices)` dimensions. The i'th value of the new vector corresponds to the value at the i'th index within the parameter pack of the original vector.
		 *
		 * Example:
		 * tz::Vec3{1.0f, 2.0f, 3.0f}.swizzle<0, 1, 0, 1>() == tz::Vec4{1.0f, 2.0f, 1.0f, 2.0f}
		 * @tparam indices Variable number of integer indices.
		 * @return Resultant swizzled vector.
		 * @pre Each element of `indices` is less than or equal to `S`, being the number of dimensions of the original vector. Otherwise, the behaviour is undefined.
		 */
		template<int... indices>
		Vector<T, sizeof...(indices)> swizzle() const;

		/**
		 * Create a new vector, with a single extra element appended to the end.
		 *
		 * Example:
		 * tz::Vec2{1.0f, 2.0f}.with_more(3.0f) == tz::Vec3{1.0f, 2.0f, 3.0f}
		 * @param end Value to be appended to the end of the resultant vector.
		 * @return Vector with extra element appended.
		 */
		Vector<T, S + 1> with_more(T&& end) const;
		/**
		 * Create a new vector, with another vector appended to the end.
		 *
		 * Example:
		 * tz::Vec2{0.0f, 1.0f}.with_more(tz::Vec3{2.0f, 3.0f, 4.0f}) == tz::Vector<float, 5>{0.0f, 1.0f, 2.0f, 3.0f, 4.0f}
		 * @tparam S2 Number of dimensions of the appendage vector.
		 * @param end Vector to be appended to the end of the resultant vector.
		 * @return Vector with extra element(s) appended.
		 */
		template<std::size_t S2>
		Vector<T, S + S2> with_more(const Vector<T, S2>& end) const;

		/**
		 * Perform a conversion to a vector of different numeric type. Numeric conversion is performed via explicit conversion.
		 */
		template<tz::Number X, typename = std::enable_if_t<!std::is_same_v<T, X>>>
		operator Vector<X, S>() const;
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
