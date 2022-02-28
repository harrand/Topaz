#include "core/assert.hpp"
#include <utility>
#include <cmath>
#include <algorithm>

namespace tz
{
	template<tz::Number T, std::size_t S>
	constexpr Vector<T, S>::Vector(std::array<T, S> data): vec(data){}

	template<tz::Number T, std::size_t S>
	const T& Vector<T, S>::operator[](std::size_t idx) const
	{
		tz_assert(idx < S, "Vector<T, %zu>::operator[%zu]: Index out of range!", S, idx);
		return this->vec[idx];
	}

	template<tz::Number T, std::size_t S>
	T& Vector<T, S>::operator[](std::size_t idx)
	{
		tz_assert(idx < S, "Vector<T, %zu>::operator[%zu]: Index out of range!", S, idx);
		return this->vec[idx];
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S>& Vector<T, S>::operator+=(const Vector<T, S>& rhs)
	{
		for(std::size_t i = 0; i < S; i++)
			this->vec[i] += rhs.vec[i];
		return *this;
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S> Vector<T, S>::operator+(const Vector<T, S>& rhs) const
	{
		Vector<T, S> copy = *this;
		copy += rhs;
		return copy;
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S>& Vector<T, S>::operator-=(const Vector<T, S>& rhs)
	{
		for(std::size_t i = 0; i < S; i++)
			this->vec[i] -= rhs.vec[i];
		return *this;
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S> Vector<T, S>::operator-(const Vector<T, S>& rhs) const
	{
		Vector<T, S> copy = *this;
		copy -= rhs;
		return copy;
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S>& Vector<T, S>::operator*=(T scalar)
	{
		for(std::size_t i = 0; i < S; i++)
			this->vec[i] *= scalar;
		return *this;
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S> Vector<T, S>::operator*(T scalar) const
	{
		Vector<T, S> copy = *this;
		copy *= scalar;
		return copy;
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S>& Vector<T, S>::operator/=(T scalar)
	{
		for(std::size_t i = 0; i < S; i++)
			this->vec[i] /= scalar;
		return *this;
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S> Vector<T, S>::operator/(T scalar) const
	{
		Vector<T, S> copy = *this;
		copy /= scalar;
		return copy;
	}

	template<tz::Number T, std::size_t S>
	bool Vector<T, S>::operator==(const Vector<T, S>& rhs) const
	{
		for(std::size_t i = 0; i < S; i++)
		{
			if constexpr(std::is_integral_v<T>)
			{
				// If we're some integer type
				if(!std::cmp_equal(this->vec[i], rhs[i]))
				{
					return false;
				}
			}
			else
			{
				// Float or double
				if(std::fabs(this->vec[i] - rhs[i]) > std::numeric_limits<T>::epsilon())
				{
					return false;
				}
			}
		}
		return true;
	}

	template<tz::Number T, std::size_t S>
	const T* Vector<T, S>::data() const
	{
		return this->vec.data();
	}

	template<tz::Number T, std::size_t S>
	T* Vector<T, S>::data()
	{
		return this->vec.data();
	}

	template<tz::Number T, std::size_t S>
	T Vector<T, S>::dot(const Vector<T, S>& rhs) const
	{
		T sum = T();
		for(std::size_t i = 0; i < S; i++)
		{
			sum += ((*this)[i] * rhs[i]);
		}
		return sum;
	}

	template<tz::Number T, std::size_t S>
	T Vector<T, S>::length() const
	{
		T sum_squares = T();
		for(std::size_t i = 0; i < S; i++)
		{
			sum_squares += (*this)[i] * (*this)[i];
		}
		return std::sqrt(sum_squares);
	}

	template<tz::Number T, std::size_t S>
	void Vector<T, S>::normalise()
	{
		T l = this->length();
		if(l == T{}) [[unlikely]]
		{
			return;
		}
		for(auto& d : this->vec)
		{
			d /= l;
		}
	}

	template<tz::Number T, std::size_t S>
	Vector<T, S> Vector<T, S>::normalised() const
	{
		Vector<T, S> cpy = *this;
		cpy.normalise();
		return cpy;
	}

	template<tz::Number T, std::size_t S>
	template<int... indices>
	Vector<T, sizeof...(indices)> Vector<T, S>::swizzle() const
	{
		auto indices_tup = std::forward_as_tuple(indices...);
		std::array<T, sizeof...(indices)> data;
		tz::static_for<0, sizeof...(indices)>([this, &data, &indices_tup](auto idx)->void
		{
			std::size_t real_idx = std::get<idx>(indices_tup);
			tz_assert(real_idx < S, "Swizzle parameter contained index %zu, which would be out-of-bounds for the original vector of size %zu. Note that this was the %zu'th template parameter out of a total of %zu", real_idx, S, idx, std::tuple_size<decltype(indices_tup)>{});
			data[idx] = this->vec[real_idx];
		});
		return {data};
	}


	template<tz::Number T, std::size_t S>
	template<tz::Number X, typename>
	Vector<T, S>::operator Vector<X, S>() const
	{
		std::array<X, S> different_data;
		std::transform(this->vec.begin(), this->vec.end(), different_data.begin(), [](const T& t){return static_cast<X>(t);});
		return {different_data};
	}

	template<tz::Number T>
	Vector<T, 3> cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs)
	{
		Vector<T, 3> ret;
		//cx = aybz − azby
		ret[0] = (lhs[1] * rhs[2]) - (lhs[2] * rhs[1]);
		//cy = azbx − axbz
		ret[1] = (lhs[2] * rhs[0]) - (lhs[0] * rhs[2]);
		//cz = axby − aybx
		ret[2] = (lhs[0] * rhs[1]) - (lhs[1] * rhs[0]);
		return std::move(ret);
	}

}
