#include "algo/container.hpp"

namespace tz
{
    template<class T, class Allocator>
    constexpr UniqueVector<T, Allocator>::UniqueVector(): Base{}{}

    template<class T, class Allocator>
    constexpr bool UniqueVector<T, Allocator>::push_back(const T& value)
    {
        if(!this->contains_element(value))
        {
            Base::push_back(value);
            return true;
        }
        return false;
    }

    template<class T, class Allocator>
    constexpr bool UniqueVector<T, Allocator>::push_back(T&& value)
    {
        if(!this->contains_element(value))
        {
            Base::push_back(value);
            return true;
        }
        return false;
    }

    template<class T, class Allocator>
    bool UniqueVector<T, Allocator>::contains_element(const T& value) const
    {
        return tz::algo::contains_element(*this, value);
    }
}