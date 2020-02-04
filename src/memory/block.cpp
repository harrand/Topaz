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

    Block::Block(void* begin, std::size_t size): Block(begin, reinterpret_cast<void*>(reinterpret_cast<char*>(begin) + size)){}

    std::size_t Block::size() const
    {
        return byte_distance(this->begin, this->end);
    }

    OwningBlock::OwningBlock(std::size_t size): Block(std::malloc(size), size){}

    OwningBlock::OwningBlock(OwningBlock&& move): Block(move.begin, move.end)
    {
        move.begin = nullptr;
        move.end = nullptr;
    }

    OwningBlock::~OwningBlock()
    {
        std::free(this->begin);
    }

    Block OwningBlock::operator()() const
    {
        return {this->begin, this->end};
    }
}