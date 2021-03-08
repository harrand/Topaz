//
// Created by Harrand on 26/12/2019.
//

#ifndef TOPAZ_CONTIGUOUS_BLOCK_HPP
#define TOPAZ_CONTIGUOUS_BLOCK_HPP
#include <cstdint>
#include <algorithm>

namespace tz::mem
{
	/**
	 * \addtogroup tz_mem Topaz Memory Library (tz::mem)
	 * A collection of low-level abstractions around memory utilities not provided by the C++ standard library. This includes non-owning memory blocks, uniform memory-pools and more.
	 * @{
	 */

	std::size_t byte_distance(void* a, void* b);

	/**
	 * Represents a non-owning block of pre-allocated memory.
	 */
	struct Block
	{
		Block(void* begin, void* end);
		Block(void* begin, std::size_t size);
		std::size_t size() const;
		static Block null();

		void* begin;
		void* end;
	};

	/**
	 * Similar to a block, but allocates the block dynamically and uses that (RAII). Non-copyable type.
	 */
	struct AutoBlock : public Block
	{
		AutoBlock(std::size_t size);
		AutoBlock(const AutoBlock& copy) = delete;
		AutoBlock(AutoBlock&& move);
		~AutoBlock();
		AutoBlock& operator=(const AutoBlock& rhs) = delete;
	};

	/**
	 * @}
	 */
}

#endif // TOPAZ_CONTIGUOUS_BLOCK_HPP