#ifndef TOPAZ_CORE_RANDOM_HPP
#define TOPAZ_CORE_RANDOM_HPP
#include <random>

namespace tz
{
    namespace detail
    {
        std::random_device& device();
    }

    template<typename T>
    T rand();

    template<typename T>
    T range(T min, T max);
}

#include "core/random.inl"
#endif