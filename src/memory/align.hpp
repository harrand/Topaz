#ifndef TOPAZ_MEMORY_ALIGN_HPP
#define TOPAZ_MEMORY_ALIGN_HPP
#include <array>
#include <type_traits>
#include <cstdint>
#include <memory>

namespace tz::mem
{
    /**
     * \addtogroup tz_mem Topaz Memory Library (tz::mem)
     * A collection of low-level abstractions around memory utilities not provided by the C++ standard library. This includes non-owning memory blocks, uniform memory-pools and more.
     * @{
     */
    
    template<typename T, std::size_t Align>
    bool is_aligned(const T& t);

    /**
     * @}
     */
}

#include "memory/align.inl"
#endif // TOPAZ_MEMORY_ALIGN_HPP