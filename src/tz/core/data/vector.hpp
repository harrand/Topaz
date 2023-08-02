#ifndef TZ_VECTOR_HPP
#define TZ_VECTOR_HPP
#include "tz/core/types.hpp"
#include <functional>
#include <array>
#include <tuple>
#include <span>
#include <cstdint>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * @internal
	 */
	template<tz::number T, std::size_t S>
	class vector
	{
	public:
		/**
		 * Create a vector filled entirely with zeros.
		 */
		static constexpr vector<T, S> zero()
		{
			return vector<T, S>::filled(T{0});
		}

		/**
		 * Create a vector filled entirely with a given value.
		 */
		static constexpr vector<T, S> filled(T t)
		{
			std::array<T, S> values;
			std::fill(values.begin(), values.end(), t);
			return {values};
		}

		/**
		 * Construct a vector directly using a variadic parameter pack value.
		 * Static Precondition: sizeof...(Ts) == S, otherwise the program is ill-formed.
		 */
		template<tz::number... Ts>
		constexpr vector(Ts&&... ts) :
		vec({ std::forward<Ts>(ts)... }) {}
		/**
		 * Construct a vector using an existing array of appropriate size.
		 */
		constexpr vector(std::array<T, S> data);
		/**
		 * Default-constructed vectors have indeterminate values.
		 */
		vector() = default;
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
		 * @param rhs vector whose elements need be added to the current vector.
		 * @return The modified original vector.
		 */
		vector<T, S>& operator+=(const vector<T, S>& rhs);
		/**
		 * Create a copy of the current vector, add each element of the given vector to the corresponding element of the copied vector and return the result.
		 * @param rhs vector whose elemens need be added to the current vector.
		 * @return A modified copy of the current vector. The initial vector is unchanged.
		 */
		vector<T, S> operator+(const vector<T, S>& rhs) const;
		/**
		 * Subtract each element of the given vector from the corresponding element of the current vector.
		 * @param rhs vector whose elements need be subtracted from the current vector.
		 * @return The modified original vector.
		 */
		vector<T, S>& operator-=(const vector<T, S>& rhs);
		/**
		 * Create a copy of the current vector, subtract each element of the given vector from the corresponding element of the copied vector and return the result.
		 * @param rhs vector whose elemens need be subtracted from the current vector.
		 * @return A modified copy of the current vector. The initial vector is unchanged.
		 */
		vector<T, S> operator-(const vector<T, S>& rhs) const;
		/**
		 * Multiply each element of the current vector by the given value.
		 * @param scalar Value to multiply by each element of the current vector.
		 * @return The modified original vector.
		 */
		vector<T, S>& operator*=(T scalar);
		/**
		 * Create a copy of the current vector, multiply each element of the copied vector by the given value, and return the result.
		 * @param scalar Value to multiply by each element of the current vector.
		 * @return A modified copy of the current vector. The initial vector is unchanged.
		 */
		vector<T, S> operator*(T scalar) const;
		/**
		 * Divide each element of the current vector by the given value.
		 * @param scalar Value to divide by each element of the current vector.
		 * @return The modified original vector.
		 */
		vector<T, S>& operator/=(T scalar);
		/**
		 * Create a copy of the current vector, divide each element of the copied vector by the given value, and return the result.
		 * @param scalar Value to divide by each element of the current vector.
		 * @return A modified copy of the current vector. The initial vector is unchanged.
		 */
		vector<T, S> operator/(T scalar) const;

		/**
		 * Equate the current vector with the given vector.
		 * @param rhs Given vector to compare with the current vector.
		 * @return True if each element of the given vector are equal to the corresponding element of the current vector.
		 */
		bool operator==(const vector<T, S>& rhs) const;
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
		T dot(const vector<T, S>& rhs) const;
		/**
		 * Retrieve the magnitude of the current vector.
		 * @return Magnitude of the vector.
		 */
		T length() const;
		/**
		 * Normalise the vector. This is equivalent to dividing the vector by its length().
		 * Note: This modifies the current vector. To retrieve a modified copy and leave the original unchanged, see vector<T, S>::normalised().
		 */
		void normalise();
		/**
		 * Create a copy of the vector, normalise it and return the result.
		 * @return A normalised copy of the current vector.
		 */
		vector<T, S> normalised() const;

		/**
		 * Perform vector swizzling.
		 *
		 * Creates a new vector of `sizeof...(indices)` dimensions. The i'th value of the new vector corresponds to the value at the i'th index within the parameter pack of the original vector.
		 *
		 * Example:
		 * tz::vec3{1.0f, 2.0f, 3.0f}.swizzle<0, 1, 0, 1>() == tz::vec4{1.0f, 2.0f, 1.0f, 2.0f}
		 * @tparam indices Variable number of integer indices.
		 * @return Resultant swizzled vector.
		 * @pre Each element of `indices` is less than or equal to `S`, being the number of dimensions of the original vector. Otherwise, the behaviour is undefined.
		 */
		template<int... indices>
		vector<T, sizeof...(indices)> swizzle() const;

		/**
		 * Create a new vector, with a single extra element appended to the end.
		 *
		 * Example:
		 * tz::vec2{1.0f, 2.0f}.with_more(3.0f) == tz::vec3{1.0f, 2.0f, 3.0f}
		 * @param end Value to be appended to the end of the resultant vector.
		 * @return vector with extra element appended.
		 */
		vector<T, S + 1> with_more(T&& end) const;
		/**
		 * Create a new vector, with another vector appended to the end.
		 *
		 * Example:
		 * tz::vec2{0.0f, 1.0f}.with_more(tz::vec3{2.0f, 3.0f, 4.0f}) == tz::vector<float, 5>{0.0f, 1.0f, 2.0f, 3.0f, 4.0f}
		 * @tparam S2 numeric of dimensions of the appendage vector.
		 * @param end vector to be appended to the end of the resultant vector.
		 * @return vector with extra element(s) appended.
		 */
		template<std::size_t S2>
		vector<T, S + S2> with_more(const vector<T, S2>& end) const;

		/**
		 * Perform a conversion to a vector of different numeric type. Numeric conversion is performed via explicit conversion.
		 */
		template<tz::number X, typename = std::enable_if_t<!std::is_same_v<T, X>>>
		operator vector<X, S>() const;
	private:
		std::array<T, S> vec;
	};

	/**
	 * @ingroup tz_core
	 * @{
	 */

	using vec1 = vector<float, 1>;
	/// A vector of two floats.
	using vec2 = vector<float, 2>;
	/// A vector of three floats.
	using vec3 = vector<float, 3>;
	/// A vector of four floats.
	using vec4 = vector<float, 4>;


	/// A vector of two shorts.
	using vec2s = vector<short, 2>;
	/// A vector of three shorts.
	using vec3s = vector<short, 3>;
	/// A vector of four shorts.
	using vec4s = vector<short, 4>;

	/// A vector of two unsigned shorts.
	using vec2us = vector<unsigned short, 2>;
	/// A vector of three shorts.
	using vec3us = vector<unsigned short, 3>;
	/// A vector of four shorts.
	using vec4us = vector<unsigned short, 4>;

	/// A vector of two ints.
	using vec2i = vector<int, 2>;
	/// A vector of three ints.
	using vec3i = vector<int, 3>;
	/// A vector of four ints.
	using vec4i = vector<int, 4>;
	
	/// A vector of two unsigned ints.
	using vec2ui = vector<unsigned int, 2>;
	/// A vector of three unsigned ints.
	using vec3ui = vector<unsigned int, 3>;
	/// A vector of four unsigned ints.
	using vec4ui = vector<unsigned int, 4>;

	/// A vector of two 32-bit ints.
	using vec2i32 = vector<std::int32_t, 2>;
	/// A vector of three 32-bit ints.
	using vec3i32 = vector<std::int32_t, 3>;
	/// A vector of four 32-bit ints.
	using vec4i32 = vector<std::int32_t, 4>;

	/// A vector of two 32-bit unsigned ints.
	using vec2ui32 = vector<std::uint32_t, 2>;
	/// A vector of three 32-bit unsigned ints.
	using vec3ui32 = vector<std::uint32_t, 3>;
	/// A vector of four 32-bit unsigned ints.
	using vec4ui32 = vector<std::uint32_t, 4>;


	template<typename T = float>
	vector<T, 3> cross(const vector<T, 3>& lhs, const vector<T, 3>& rhs);

	/**
	 * @}
	 */
}

namespace std
{
	template<tz::number T, std::size_t S>
	struct hash<tz::vector<T, S>>
	{
		std::size_t operator()(const tz::vector<T, S>& v) const
		{
			std::size_t ret = 0;
			for(const T& t : v.data())
			{
				ret |= std::hash<T>{}(t);
			}
			return ret;
		}
	};
}

#include "tz/core/data/vector.inl"
#endif // TZ_VECTOR_HPP
