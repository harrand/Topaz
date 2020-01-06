#ifndef TOPAZ_MEMORY_ALIGN_HPP
#define TOPAZ_MEMORY_ALIGN_HPP
#include <array>
#include <type_traits>
#include <cstdint>
#include <memory>

namespace tz::mem
{
    template<typename T, std::size_t Align>
    bool is_aligned(const T& t);

    template<typename T, std::size_t Align, std::size_t Quantity>
    using AlignedTypeArray = alignas(Align) T[Quantity];
}

#include "memory/align.inl"
#endif // TOPAZ_MEMORY_ALIGN_HPP