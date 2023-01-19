#ifndef TOPAZ_CORE_ALLOCATORS_NULL_HPP
#define TOPAZ_CORE_ALLOCATORS_NULL_HPP
#include "tz/core/memory/memblk.hpp"
#include "tz/core/types.hpp"

namespace tz
{
	/**
	 * @ingroup tz_memory_allocator
	 * An allocator which always returns nullptr.
	 */
	class null_allocator
	{
	public:
		constexpr null_allocator() = default;
		constexpr tz::memblk allocate(std::size_t count) const{return tz::nullblk;}
		constexpr void deallocate(tz::memblk blk) const{}
		constexpr bool owns(tz::memblk blk) const{return blk == tz::nullblk;}
	};

	static_assert(tz::allocator<null_allocator>);
}

#endif // TOPAZ_CORE_ALLOCATORS_NULL_HPP
