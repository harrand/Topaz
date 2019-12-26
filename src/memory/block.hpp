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

    class Block
    {
    public:
        Block(void* begin, void* end);
        std::size_t size() const;
    private:
        void* begin;
        void* end;
    };
}

#endif // TOPAZ_CONTIGUOUS_BLOCK_HPP