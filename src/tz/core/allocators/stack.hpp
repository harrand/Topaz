#ifndef TOPAZ_CORE_ALLOCATORS_STACK_HPP
#define TOPAZ_CORE_ALLOCATORS_STACK_HPP
#include "tz/core/allocators/linear.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_allocators
	 * An allocator which has its own fixed-size buffer on the stack from which memory is sub-allocated. Aside from operating on its own stack arena, a stack allocator behaves identically to a @ref LinearAllocator.
	 *
	 * StackAllocators are excellent candidates for fallback allocators. `FallbackAllocator<StackAllocator<X>, Mallocator>` allows you to have small-size optimisation for free, for example.
	 * @tparam S Size of local automatic storage.
	 */
	template<std::size_t S>
	class StackAllocator : public LinearAllocator
	{
	public:
		StackAllocator();
	private:
		alignas(alignof(std::max_align_t)) char data[S];
	};

	static_assert(tz::Allocator<StackAllocator<1>>);
}

#include "tz/core/allocators/stack.inl"
#endif // TOPAZ_CORE_ALLOCATORS_STACK_HPP
