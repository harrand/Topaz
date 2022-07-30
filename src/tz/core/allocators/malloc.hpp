#ifndef TOPAZ_CORE_ALLOCATORS_MALLOC_HPP
#define TOPAZ_CORE_ALLOCATORS_MALLOC_HPP
#include "tz/core/types.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_allocators
	 * Implements @ref tz:Allocator
	 *
	 * An allocator which simply calls malloc. It thinks it owns all memory, so if you're using this in a @ref tz::FallbackAllocator make sure it is always used as a secondary allocator, never the primary (or you will `free()` no matter what).
	 */
	class Mallocator
	{
	public:
		constexpr Mallocator() = default;
		tz::Blk allocate(std::size_t count) const;
		void deallocate(tz::Blk blk) const;
		bool owns(tz::Blk blk) const;
	};

	static_assert(tz::Allocator<Mallocator>);
}

#endif // TOPAZ_CORE_ALLOCATORS_MALLOC_HPP
