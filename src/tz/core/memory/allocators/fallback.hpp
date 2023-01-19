#ifndef TOPAZ_CORE_ALLOCATORS_FALLBACK_HPP
#define TOPAZ_CORE_ALLOCATORS_FALLBACK_HPP
#include "tz/core/memory/memblk.hpp"
#include "tz/core/types.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_memory_allocator
	 * Implements @ref tz::allocator
	 *
	 * A meta-allocator which will use a primary allocator, but fall-back to a secondary allocator upon failure. fallback_allocators can be chained together.
	 *
	 * Often, @ref tz::Mallocator and @ref tz::Nullallocator are useful secondary allocators.
	 * @tparam P Primary allocator type. This will always be used first.
	 * @tparam S Secondary allocator type. This will only be used if the primary allocator fails (such as returning a nullblk on allocate(...))
	 */
	template<tz::allocator P, tz::allocator S>
	class fallback_allocator : private P, private S
	{
	public:
		tz::memblk allocate(std::size_t count)
		{
			tz::memblk r = P::allocate(count);
			if(r.ptr == nullptr)
			{
				r = S::allocate(count);
			}
			return r;
		}

		void deallocate(tz::memblk blk)
		{
			if(P::owns(blk))
			{
				P::deallocate(blk);
			}
			else
			{
				S::deallocate(blk);
			}
		}

		bool owns(tz::memblk blk) const
		{
			return P::owns(blk) || S::owns(blk);
		}
	};
}

#endif // TOPAZ_CORE_ALLOCATORS_FALLBACK_HPP
