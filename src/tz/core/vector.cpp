#include "tz/core/vector.hpp"

namespace tz
{
#define VECTOR_TEMPLATE_IMPL \
    template<typename T, int N> \
    requires std::integral<T> || std::floating_point<T>

	VECTOR_TEMPLATE_IMPL
	const T& vector<T, N>::operator[](std::size_t idx) const
	{
		return this->arr[idx];
	}

	VECTOR_TEMPLATE_IMPL
	T& vector<T, N>::operator[](std::size_t idx)
	{
		return this->arr[idx];
	}

	VECTOR_TEMPLATE_IMPL
	vector<T, N>& vector<T, N>::operator+=(const vector<T, N>& rhs)
	{
		for(std::size_t i = 0; i < N; i++)
		{
			this->arr[i] += rhs.arr[i];
		}
		return *this;
	}

	VECTOR_TEMPLATE_IMPL
	vector<T, N>& vector<T, N>::operator-=(const vector<T, N>& rhs)
	{
		for(std::size_t i = 0; i < N; i++)
		{
			this->arr[i] -= rhs.arr[i];
		}
		return *this;
	}

	VECTOR_TEMPLATE_IMPL
	vector<T, N>& vector<T, N>::operator*=(const vector<T, N>& rhs)
	{
		for(std::size_t i = 0; i < N; i++)
		{
			this->arr[i] *= rhs.arr[i];
		}
		return *this;
	}

	VECTOR_TEMPLATE_IMPL
	vector<T, N>& vector<T, N>::operator/=(const vector<T, N>& rhs)
	{
		for(std::size_t i = 0; i < N; i++)
		{
			this->arr[i] /= rhs.arr[i];
		}
		return *this;
	}

    template struct vector<int, 2>;
	template struct vector<int, 3>;
	template struct vector<int, 4>;
    template struct vector<unsigned int, 2>;
	template struct vector<unsigned int, 3>;
	template struct vector<unsigned int, 4>;
    template struct vector<float, 2>;
	template struct vector<float, 3>;
	template struct vector<float, 4>;
    template struct vector<double, 2>;
	template struct vector<double, 3>;
	template struct vector<double, 4>;
}