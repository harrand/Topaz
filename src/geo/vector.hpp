#ifndef TOPAZ_GEOMETRY_VECTOR_HPP
#define TOPAZ_GEOMETRY_VECTOR_HPP
#include "memory/align.hpp"
#include <xmmintrin.h>

namespace tz::geo
{
    namespace sse
    {
        template<typename T, std::size_t Quantity>
        using SSEValidArray = tz::mem::AlignedTypeArray<T, 16, Quantity>;

        using FloatHandle = __m128;

        template<std::size_t Quantity>
        FloatHandle* handle_array(SSEValidArray<float, Quantity>& aligned_data);
    }
}

#include "geo/vector.inl"
#endif // TOPAZ_GEOMETRY_VECTOR_HPP