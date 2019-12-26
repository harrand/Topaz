//
// Created by Harrand on 26/12/2019.
//

#ifndef TOPAZ_CONTIGUOUS_BLOCK_HPP
#define TOPAZ_CONTIGUOUS_BLOCK_HPP
#include <cstdint>
#include <algorithm>

namespace tz::mem
{
    std::size_t byte_distance(void* a, void* b);

    struct Block
    {
        Block(void* begin, void* end);
        Block(void* begin, std::size_t size);
        std::size_t size() const;

        void* begin;
        void* end;
    };
}

#endif // TOPAZ_CONTIGUOUS_BLOCK_HPP