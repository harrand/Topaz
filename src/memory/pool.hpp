//
// Created by Harrand on 20/12/2019.
//

#ifndef TOPAZ_CONTIGUOUS_BLOCK_HPP
#define TOPAZ_CONTIGUOUS_BLOCK_HPP
#include <cstdint>

namespace tz::mem
{
	/**
	 * Manages a pre-allocated block of memory, treating it as a contiguous array of Ts.
	 * @tparam T
	 */
	template<typename T>
	class UniformPool
	{
	public:
		UniformPool(void* begin, void* end);
		UniformPool(void* begin, std::size_t size_bytes);
	private:
		void* begin;
		std::size_t size_bytes;
	};
}

#include "memory/pool.inl"
#endif //TOPAZ_CONTIGUOUS_BLOCK_HPP
