#include "tz/core/vector.hpp"
#include <cmath>

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
	vector<T, N>& vector<T, N>::operator*=(T scalar)
	{
		for(std::size_t i = 0; i < N; i++)
		{
			this->arr[i] *= scalar;
		}
		return *this;
	}

	VECTOR_TEMPLATE_IMPL
	vector<T, N>& vector<T, N>::operator/=(T scalar)
	{
		for(std::size_t i = 0; i < N; i++)
		{
			this->arr[i] /= scalar;
		}
		return *this;
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

	VECTOR_TEMPLATE_IMPL
	T vector<T, N>::length() const
	{
		T ret = T{0};
		for(std::size_t i = 0; i < N; i++)
		{
			ret += ((*this)[i] * (*this)[i]);
		}
		return std::sqrt(ret);
	}

	VECTOR_TEMPLATE_IMPL
	T vector<T, N>::dot(const vector<T, N>& rhs) const
	{
		T ret = T{0};
		for(std::size_t i = 0; i < N; i++)
		{
			ret += (*this)[i] * rhs[i];
		}
		return ret;
	}

	VECTOR_TEMPLATE_IMPL
	vector<T, N> vector<T, N>::cross(const vector<T, N>& rhs) const requires(N == 3)
	{
		vector<T, 3> ret;
		//cx = aybz − azby
		ret[0] = ((*this)[1] * rhs[2]) - ((*this)[2] * rhs[1]);
		//cy = azbx − axbz
		ret[1] = ((*this)[2] * rhs[0]) - ((*this)[0] * rhs[2]);
		//cz = axby − aybx
		ret[2] = ((*this)[0] * rhs[1]) - ((*this)[1] * rhs[0]);
		return ret;
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