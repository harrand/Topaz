#ifndef TOPAZ_CORE_ALLOCATORS_STACK_HPP
#define TOPAZ_CORE_ALLOCATORS_STACK_HPP
#include "core/types.hpp"
#include <functional>
#include <cstddef>

namespace tz
{
	/**
	 * @ingroup tz_core_allocators
	 * Implements @ref tz::Allocator
	 *
	 * An allocator which comes with a fixed-size pre-allocated automatic storage for use in allocations. If the storage is exhausted, allocations will begin to fail, returning the null block.
	 *
	 * StackAllocators are excellent candidates for fallback allocators. `FallbackAllocator<StackAllocator<X>, Mallocator>` allows you to have small-size optimisation for free, for example.
	 * @tparam S Size of local automatic storage.
	 */
	template<std::size_t S>
	class StackAllocator
	{
	public:
		tz::Blk allocate(std::size_t count);
		void deallocate(tz::Blk blk);
		bool owns(tz::Blk blk) const;
	private:
		char* head();

		alignas(alignof(std::max_align_t)) char data[S];
		std::size_t offset = 0;
	};

	static_assert(tz::Allocator<StackAllocator<1>>);
}

#include "core/allocators/stack.inl"
#endif // TOPAZ_CORE_ALLOCATORS_STACK_HPP
