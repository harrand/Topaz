#include "core/debug/assert.hpp"
#include <utility>
#include <cmath>

namespace tz
{
    template<typename T, std::size_t S>
    constexpr Vector<T, S>::Vector(std::array<T, S> data): vec(data){}

    template<typename T, std::size_t S>
    const T& Vector<T, S>::operator[](std::size_t idx) const
    {
        topaz_assert(idx < S, "Vector<T, ", S, ">::operator[", idx, "]: Index out of range!");
        return this->vec[idx];
    }

    template<typename T, std::size_t S>
    T& Vector<T, S>::operator[](std::size_t idx)
    {
        topaz_assert(idx < S, "Vector<T, ", S, ">::operator[", idx, "]: Index out of range!");
        return this->vec[idx];
    }

    template<typename T, std::size_t S>
    Vector<T, S>& Vector<T, S>::operator+=(const Vector<T, S>& rhs)
    {
        for(std::size_t i = 0; i < S; i++)
            this->vec[i] += rhs.vec[i];
        return *this;
    }

    template<typename T, std::size_t S>
    Vector<T, S> Vector<T, S>::operator+(const Vector<T, S>& rhs) const
    {
        Vector<T, S> copy = *this;
        copy += rhs;
        return std::move(copy);
    }

    template<typename T, std::size_t S>
    Vector<T, S>& Vector<T, S>::operator-=(const Vector<T, S>& rhs)
    {
        for(std::size_t i = 0; i < S; i++)
            this->vec[i] -= rhs.vec[i];
        return *this;
    }

    template<typename T, std::size_t S>
    Vector<T, S> Vector<T, S>::operator-(const Vector<T, S>& rhs) const
    {
        Vector<T, S> copy = *this;
        copy -= rhs;
        return std::move(copy);
    }

    template<typename T, std::size_t S>
    Vector<T, S>& Vector<T, S>::operator*=(T scalar)
    {
        for(std::size_t i = 0; i < S; i++)
            this->vec[i] *= scalar;
        return *this;
    }

    template<typename T, std::size_t S>
    Vector<T, S> Vector<T, S>::operator*(T scalar) const
    {
        Vector<T, S> copy = *this;
        copy *= scalar;
        return std::move(copy);
    }

    template<typename T, std::size_t S>
    Vector<T, S>& Vector<T, S>::operator/=(T scalar)
    {
        for(std::size_t i = 0; i < S; i++)
            this->vec[i] /= scalar;
        return *this;
    }

    template<typename T, std::size_t S>
    Vector<T, S> Vector<T, S>::operator/(T scalar) const
    {
        Vector<T, S> copy = *this;
        copy /= scalar;
        return std::move(copy);
    }

    template<typename T, std::size_t S>
    T Vector<T, S>::dot(const Vector<T, S>& rhs) const
    {
        T sum = T();
        for(std::size_t i = 0; i < S; i++)
        {
            sum += ((*this)[i] * rhs[i]);
        }
        return sum;
    }

    template<typename T, std::size_t S>
    T Vector<T, S>::length() const
    {
        T sum_squares = T();
        for(std::size_t i = 0; i < S; i++)
        {
            sum_squares += (*this)[i] * (*this)[i];
        }
        return std::sqrt(sum_squares);
    }

}