#ifndef TOPAZ_CORE_RANDOM_HPP
#define TOPAZ_CORE_RANDOM_HPP
#include <random>

namespace tz
{
    namespace detail
    {
        std::random_device& device();
    }

    /**
     * Retrieve a random value.
     * @tparam T Type of value to generate. This should be implicitly convertible from `unsigned int`.
     * @return Any possible value of T.
     */
    template<typename T>
    T rand();

    /**
     * Retrieve a random value within a given range (uniformly).
     * @tparam T Type of value to generate. This should be implicitly convertible from `unsigned int`.
     * @param min Minimum value which can be generated.
     * @param max Maximum value which can be generated.
     * @return Any possible value of T that is between min and max (double-inclusive).
     */
    template<typename T>
    T range(T min, T max);
}

#include "core/random.inl"
#endif