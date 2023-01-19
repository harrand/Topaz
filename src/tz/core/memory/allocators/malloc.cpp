#include "tz/core/memory/allocators/malloc.hpp"
#include <cstdlib>

namespace tz
{
	tz::memblk mallocator::allocate(std::size_t count) const
	{
		void* addr = std::malloc(count);
		if(addr == nullptr)
		{
			return tz::nullblk;
		}
		return
		{
			.ptr = addr,
			.size = count
		};
	}

	void mallocator::deallocate(tz::memblk blk) const
	{
		std::free(blk.ptr);
	}

	bool mallocator::owns([[maybe_unused]] tz::memblk blk) const
	{
		return true;
	}
}
