#ifndef TOPAZ_ALGO_CONTAINER_HPP
#define TOPAZ_ALGO_CONTAINER_HPP
#include <cstddef>

namespace tz::algo
{
    template<typename StandardContainer>
    constexpr std::size_t sizeof_element(const StandardContainer& container);

    template<typename StandardContainer>
    constexpr std::size_t sizeof_element();
}

#include "algo/container.inl"
#endif // TOPAZ_ALGO_CONTAINER_HPP