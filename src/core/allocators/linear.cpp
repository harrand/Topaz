#include "core/allocators/linear.hpp"
#include <functional>

namespace tz
{
	LinearAllocator::LinearAllocator(std::span<std::byte> arena):
	arena(arena){}

	LinearAllocator::LinearAllocator(tz::Blk arena):
	LinearAllocator(std::span<std::byte>{static_cast<std::byte*>(arena.ptr), arena.size}){}

	tz::Blk LinearAllocator::allocate(std::size_t count)
	{
		std::byte* h = this->head();
		std::size_t compulsive_size = std::max(count, alignof(std::max_align_t));
		// No longer care about alignment.
		tz::Blk ret{.ptr = h, .size = count};
		if(this->cursor + compulsive_size > this->arena.size_bytes())
		{
			return tz::nullblk;
		}
		this->cursor += compulsive_size;
		return ret;
	}

	void LinearAllocator::deallocate(tz::Blk blk)
	{
		std::size_t compulsive_size = std::max(blk.size, alignof(std::max_align_t));
		if(this->cursor < compulsive_size)
		{
			return;
		}
		auto after_blk = static_cast<std::byte*>(blk.ptr);
		std::advance(after_blk, compulsive_size);
		if(after_blk == this->head())
		{
			// This was the most recently allocated block, let's "deallocate" it.
			this->cursor -= compulsive_size;
		}
		// Otherwise we literally do nothing.
	}

	bool LinearAllocator::owns(tz::Blk blk) const
	{
		return std::greater_equal<const void*>{}(blk.ptr, this->arena.data()) && std::less_equal<const void*>{}(blk.ptr, this->arena.data() + this->arena.size_bytes());
	}

	std::byte* LinearAllocator::head()
	{
		return this->arena.data() + this->cursor;
	}
}
