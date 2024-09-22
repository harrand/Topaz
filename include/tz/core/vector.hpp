#ifndef TOPAZ_CORE_VECTOR_HPP
#define TOPAZ_CORE_VECTOR_HPP
#include <concepts>
#include <array>

namespace tz
{
	template<typename T, int N>
	requires std::integral<T> || std::floating_point<T>
	struct vector
	{
		static constexpr vector<T, N> zero()
		{
			return vector<T, N>::filled(T{0});
		}

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

		vector<T, N>& operator+=(const vector<T, N>& rhs);
		vector<T, N>& operator-=(const vector<T, N>& rhs);
		vector<T, N>& operator*=(const vector<T, N>& rhs);
		vector<T, N>& operator/=(const vector<T, N>& rhs);

		vector<T, N> operator+(const vector<T, N>& rhs){auto cpy = *this; return cpy += rhs;}
		vector<T, N> operator-(const vector<T, N>& rhs){auto cpy = *this; return cpy -= rhs;}
		vector<T, N> operator*(const vector<T, N>& rhs){auto cpy = *this; return cpy *= rhs;}
		vector<T, N> operator/(const vector<T, N>& rhs){auto cpy = *this; return cpy /= rhs;}

		bool operator==(const vector<T, N>& rhs) const = default;
	private:
		std::array<T, N> arr;
	};

	using v2i = vector<int, 2>;
	using v3i = vector<int, 3>;
	using v4i = vector<int, 4>;
	using v2u = vector<unsigned int, 2>;
	using v3u = vector<unsigned int, 3>;
	using v4u = vector<unsigned int, 4>;
	using v2f = vector<float, 2>;
	using v3f = vector<float, 3>;
	using v4f = vector<float, 4>;
	using v2d = vector<double, 2>;
	using v3d = vector<double, 3>;
	using v4d = vector<double, 4>;
}

#endif // TOPAZ_CORE_VECTOR_HPP