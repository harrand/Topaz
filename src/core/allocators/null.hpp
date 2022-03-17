#ifndef TOPAZ_CORE_ALLOCATORS_NULL_HPP
#define TOPAZ_CORE_ALLOCATORS_NULL_HPP
#include "core/types.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_allocators
	 * An allocator which always returns nullptr.
	 */
	class NullAllocator
	{
	public:
		constexpr NullAllocator() = default;
		constexpr tz::Blk allocate(std::size_t count) const{return tz::nullblk;}
		constexpr void deallocate(tz::Blk blk) const{}
		constexpr bool owns(tz::Blk blk) const{return blk == tz::nullblk;}
	};

	static_assert(tz::Allocator<NullAllocator>);
}

#endif // TOPAZ_CORE_ALLOCATORS_NULL_HPP
