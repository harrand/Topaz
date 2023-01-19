#ifndef TOPAZ_CORE_ALLOCATORS_MALLOC_HPP
#define TOPAZ_CORE_ALLOCATORS_MALLOC_HPP
#include "tz/core/memory/memblk.hpp"
#include "tz/core/types.hpp"

namespace tz
{
	/**
	 * @ingroup tz_memory_allocator
	 * Implements @ref tz:allocator
	 *
	 * An allocator which simply calls malloc. It thinks it owns all memory, so if you're using this in a @ref tz::fallback_allocator make sure it is always used as a secondary allocator, never the primary (or you will `free()` no matter what).
	 */
	class mallocator
	{
	public:
		constexpr mallocator() = default;
		tz::memblk allocate(std::size_t count) const;
		void deallocate(tz::memblk blk) const;
		bool owns(tz::memblk blk) const;
	};

	static_assert(tz::allocator<mallocator>);
}

#endif // TOPAZ_CORE_ALLOCATORS_MALLOC_HPP
