//
// Created by Harrand on 26/12/2019.
//

#include "memory/block.hpp"

namespace tz::mem
{
    std::size_t byte_distance(void* a, void* b)
    {
        std::ptrdiff_t signed_distance = std::distance(reinterpret_cast<char*>(a), reinterpret_cast<char*>(b));
        return static_cast<std::size_t>(std::abs(signed_distance));
    }

    Block::Block(void* begin, void* end): begin(begin), end(end){}

    std::size_t Block::size() const
    {
        return byte_distance(this->begin, this->end);
    }
}