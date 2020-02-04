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

    /**
     * Represents a non-owning contiguous block of memory.
     */
    struct Block
    {
        Block(void* begin, void* end);
        Block(void* begin, std::size_t size);
        std::size_t size() const;
        template<typename T = char>
        T* get(std::size_t idx);
        template<typename T = char>
        const T* get(std::size_t idx) const;

        void* begin;
        void* end;
    };

    /**
     * Represents an owning block of contiguous memory.
     */
    struct OwningBlock : public Block
    {
        OwningBlock(std::size_t size);
        OwningBlock(const OwningBlock& copy) = delete;
        OwningBlock(OwningBlock&& move);
        ~OwningBlock();
        /**
         * Retrieve a non-owning block managing the same memory as this current block.
         * 
         * Does not cause any changes to current ownership semantics -- It is UB to use the new pool after the lifetime of this owning pool has ended.
         */
        Block operator()() const;
    };
}

#include "memory/block.inl"
#endif // TOPAZ_CONTIGUOUS_BLOCK_HPP