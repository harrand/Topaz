#include "tz/core/allocators/malloc.hpp"
#include <cstdlib>

namespace tz
{
	tz::Blk Mallocator::allocate(std::size_t count) const
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

	void Mallocator::deallocate(tz::Blk blk) const
	{
		std::free(blk.ptr);
	}

	bool Mallocator::owns([[maybe_unused]] tz::Blk blk) const
	{
		return true;
	}
}
