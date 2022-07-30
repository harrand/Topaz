#ifndef TOPAZ_CORE_ALLOCATORS_FALLBACK_HPP
#define TOPAZ_CORE_ALLOCATORS_FALLBACK_HPP
#include "tz/core/types.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_allocators
	 * Implements @ref tz::Allocator
	 *
	 * A meta-allocator which will use a primary allocator, but fall-back to a secondary allocator upon failure. FallbackAllocators can be chained together.
	 *
	 * Often, @ref tz::Mallocator and @ref tz::NullAllocator are useful secondary allocators.
	 * @tparam Primary Primary allocator type. This will always be used first.
	 * @tparam Secondary Secondary allocator type. This will only be used if the primary allocator fails (such as returning a nullblk on allocate(...))
	 */
	template<tz::Allocator Primary, tz::Allocator Secondary>
	class FallbackAllocator : private Primary, private Secondary
	{
	public:
		tz::Blk allocate(std::size_t count)
		{
			tz::Blk r = Primary::allocate(count);
			if(r.ptr == nullptr)
			{
				r = Secondary::allocate(count);
			}
			return r;
		}

		void deallocate(tz::Blk blk)
		{
			if(Primary::owns(blk))
			{
				Primary::deallocate(blk);
			}
			else
			{
				Secondary::deallocate(blk);
			}
		}

		bool owns(tz::Blk blk) const
		{
			return Primary::owns(blk) || Secondary::owns(blk);
		}
	};
}

#endif // TOPAZ_CORE_ALLOCATORS_FALLBACK_HPP
