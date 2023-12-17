#include "tz/core/memory/allocators/slab.hpp"

namespace tz
{
	tz::memblk slab_allocator::allocate(std::size_t sz)
	{
		tz::memblk ret = tz::nullblk;
		// go through our slabs and try to allocate.
		for(auto& slab : this->slabs)
		{
			ret = slab.alloc.allocate(sz);
			if(ret != tz::nullblk)
			{
				return ret;
			}
		}

		// if we still have a nullblk, everyone failed. and we gotta make a new slab.
		ret = this->new_slab(sz).alloc.allocate(sz);
		return ret;
	}

	void slab_allocator::deallocate(tz::memblk blk)
	{
		for(auto& slab : this->slabs)
		{
			if(slab.alloc.owns(blk))
			{
				slab.alloc.deallocate(blk);
				return;
			}
		}
	}

	bool slab_allocator::owns(tz::memblk blk) const
	{
		for(const auto& slab : this->slabs)
		{
			if(slab.alloc.owns(blk))
			{
				return true;
			}
		}
		return false;
	}

	slab_allocator::slab_t::slab_t(std::size_t sz):
	mem(std::make_unique_for_overwrite<char[]>(sz)),
	alloc({.ptr = mem.get(), .size = sz}),
	sz(sz)
	{}

	slab_allocator::slab_t::slab_t(const slab_t& cpy):
	slab_t(cpy.sz)
	{}

	slab_allocator::slab_t& slab_allocator::slab_t::operator=(const slab_t& cpy)
	{
		// does nothing.
		(void)cpy;
		return *this;
	}

	slab_allocator::slab_t& slab_allocator::new_slab(std::size_t sz)
	{
		// allocate either the size requested or a magic number, whichever is greater.
		// we prefer page sizes, but to handle massive allocations we need to factor in the sz too.
		this->slabs.push_back(std::max(sz, static_cast<std::size_t>(1024 * 1024)));
		return this->slabs.back();
	}
}