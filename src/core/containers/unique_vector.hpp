#ifndef TOPAZ_CORE_UNIQUE_VECTOR_HPP
#define TOPAZ_CORE_UNIQUE_VECTOR_HPP
#include <vector>
#include <memory>

namespace tz::core
{
    template<class T, class Allocator = std::allocator<T>>
    class UniqueVector : public std::vector<T, Allocator>
    {
    public:
        using Base = std::vector<T, Allocator>;
        constexpr UniqueVector();
        // Shadows base
        constexpr bool push_back(const T& value);
        constexpr bool push_back(T&& value);
    private:
        bool contains_element(const T& value) const;
    };
}

#include "core/containers/unique_vector.inl"
#endif // TOPAZ_CORE_UNIQUE_VECTOR_HPP