#ifndef TOPAZ_CORE_ALLOCATORS_LINEAR_HPP
#define TOPAZ_CORE_ALLOCATORS_LINEAR_HPP
#include "core/types.hpp"
#include <span>
#include <cstddef>

namespace tz
{
	/**
	 * @ingroup tz_core_allocators
	 * An allocator which operates on a pre-allocated buffer of variable size.
	 *
	 * On construction, the arena buffer is considered to be wholly uninitialised. Each allocationslowly fills the arena until there is not enough space for an allocation request, in which case the null block is returned.
	 */
	class LinearAllocator
	{
	public:
		LinearAllocator(std::span<std::byte> arena);
		LinearAllocator(tz::Blk arena);

		tz::Blk allocate(std::size_t count);
		void deallocate(tz::Blk blk);
		bool owns(tz::Blk blk) const;
	private:
		std::byte* head();

		std::span<std::byte> arena;
		std::size_t cursor = 0;
	};

	static_assert(tz::Allocator<LinearAllocator>);
}

#endif // TOPAZ_CORE_ALLOCATORS_LINEAR_HPP
