#include "core/debug/assert.hpp"
#include <utility>
#include <cmath>

namespace tz
{
	template<tz::Number T, std::size_t S>
	template<typename... Ts, typename>
	constexpr Vector<T, S>::Vector(Ts&&... ts): vec({std::forward<Ts>(ts)...})
	{
		static_assert(sizeof...(Ts) == S, "tz::Vector<T, S>::Vector<sizeof...(Ts)>(...): Constructor param pack size does not match vector width.");
	}

	template<tz::Number T, std::size_t S>
	constexpr Vector<T, S>::Vector(std::array<T, S> data): vec(data){}

	template<tz::Number T, std::size_t S>
	const T& Vector<T, S>::operator[](std::size_t idx) const
	{
		topaz_assert(idx < S, "Vector<T, ", S, ">::operator[", idx, "]: Index out of range!");
		return this->vec[idx];
	}

	template<tz::Number T, std::size_t S>
	T& Vector<T, S>::operator[](std::size_t idx)
	{
		topaz_assert(idx < S, "Vector<T, ", S, ">::operator[", idx, "]: Index out of range!");
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
			if(std::abs((*this)[i] - rhs[i]) >= std::numeric_limits<T>::epsilon())
				return false;
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