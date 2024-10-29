#ifndef TOPAZ_CORE_VECTOR_HPP
#define TOPAZ_CORE_VECTOR_HPP
#include <concepts>
#include <array>
#include <type_traits>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_math Mathematics
	 * @brief Basic mathematical geometric types.
	 */

	/**
	 * @ingroup tz_core_math
	 * @brief Represents the generic vector.
	 * @internal
	 */
	template<typename T, int N>
	requires std::integral<T> || std::floating_point<T>
	struct vector
	{
		/// Retrieve a vector filled with zeroes.
		static constexpr vector<T, N> zero()
		{
			return vector<T, N>::filled(T{0});
		}

		/// Retrieve a vector filled with the given value.
		static constexpr vector<T, N> filled(T t)
		{
			std::array<T, N> values;
			std::fill(values.begin(), values.end(), t);
			return {values};
		}

		/**
		 * Construct a vector directly using a variadic parameter pack value.
		 * If the number of parameters provided is less than the vector's length, the remainder elements of the vector will be of indeterminate value.
		 */
		template<typename... Ts>
		requires(
			sizeof...(Ts) <= N
			&& sizeof...(Ts) > 0
			&& std::is_convertible_v<std::tuple_element_t<0, std::tuple<Ts...>>, T>)
		constexpr vector(Ts&&... ts) :
		arr({ std::forward<Ts>(ts)... }) {}
		/**
		 * Construct a vector using an existing array of appropriate size.
		 */
		constexpr vector(std::array<T, N> data): arr(data){}
		/**
		 * Default-constructed vectors have indeterminate values.
		 */
		vector() = default;
		vector(const vector<T, N>& cpy) = default;
		vector(vector<T, N>&& move) = default;
		vector& operator=(const vector<T, N>& rhs) = default;
		vector& operator=(vector<T, N>&& rhs) = default;

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

		/// Add one vector to another.
		vector<T, N>& operator+=(const vector<T, N>& rhs);
		/// Subtract one vector from another.
		vector<T, N>& operator-=(const vector<T, N>& rhs);
		/// Multiply one vector with another.
		vector<T, N>& operator*=(const vector<T, N>& rhs);
		/// Divide one vector by another.
		vector<T, N>& operator/=(const vector<T, N>& rhs);

		/// Add one vector to another.
		vector<T, N> operator+(const vector<T, N>& rhs) const{auto cpy = *this; return cpy += rhs;}
		/// Subtract one vector from another.
		vector<T, N> operator-(const vector<T, N>& rhs) const{auto cpy = *this; return cpy -= rhs;}
		/// Multiply one vector with another.
		vector<T, N> operator*(const vector<T, N>& rhs) const{auto cpy = *this; return cpy *= rhs;}
		/// Divide one vector by another.
		vector<T, N> operator/(const vector<T, N>& rhs) const{auto cpy = *this; return cpy /= rhs;}

		/// Retrieve the magnitude of the vector.
		T length() const;
		/// Retrieve the dot (scalar) product of two vectors.
		T dot(const vector<T, N>& rhs) const;
		/// Retrieve a cross product between two three-dimensional vectors. @warning If you invoke this on a vector that is not three-dimensional, the program is ill-formed.
		vector<T, N> cross(const vector<T, N>& rhs) const requires(N == 3);

		/// Compare two vectors. Two vectors are equal if all of their components are exactly equal.
		bool operator==(const vector<T, N>& rhs) const = default;
	private:
		std::array<T, N> arr;
	};

	/**
	 * @ingroup tz_core_math
	 * @brief 2d integer vector.
	 */
	using v2i = vector<int, 2>;
	/**
	 * @ingroup tz_core_math
	 * @brief 3d integer vector.
	 */
	using v3i = vector<int, 3>;
	/**
	 * @ingroup tz_core_math
	 * @brief 4d integer vector.
	 */
	using v4i = vector<int, 4>;
	/**
	 * @ingroup tz_core_math
	 * @brief 2d unsigned integer vector.
	 */
	using v2u = vector<unsigned int, 2>;
	/**
	 * @ingroup tz_core_math
	 * @brief 3d unsigned integer vector.
	 */
	using v3u = vector<unsigned int, 3>;
	/**
	 * @ingroup tz_core_math
	 * @brief 4d unsigned integer vector.
	 */
	using v4u = vector<unsigned int, 4>;
	/**
	 * @ingroup tz_core_math
	 * @brief 2d single-precision floating-point vector.
	 */
	using v2f = vector<float, 2>;
	/**
	 * @ingroup tz_core_math
	 * @brief 3d single-precision floating-point vector.
	 */
	using v3f = vector<float, 3>;
	/**
	 * @ingroup tz_core_math
	 * @brief 4d single-precision floating-point vector.
	 */
	using v4f = vector<float, 4>;
	/**
	 * @ingroup tz_core_math
	 * @brief 2d double-precision floating-point vector.
	 */
	using v2d = vector<double, 2>;
	/**
	 * @ingroup tz_core_math
	 * @brief 3d double-precision floating-point vector.
	 */
	using v3d = vector<double, 3>;
	/**
	 * @ingroup tz_core_math
	 * @brief 4d double-precision floating-point vector.
	 */
	using v4d = vector<double, 4>;
}

#endif // TOPAZ_CORE_VECTOR_HPP