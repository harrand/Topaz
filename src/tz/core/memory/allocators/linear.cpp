#include "tz/core/memory/allocators/linear.hpp"
#include <functional>

namespace tz
{
	linear_allocator::linear_allocator(std::span<std::byte> arena):
	arena(arena){}

	linear_allocator::linear_allocator(tz::memblk arena):
	linear_allocator(std::span<std::byte>{static_cast<std::byte*>(arena.ptr), arena.size}){}

	tz::memblk linear_allocator::allocate(std::size_t count)
	{
		std::byte* h = this->head();
		std::size_t compulsive_size = std::max(count, alignof(std::max_align_t));
		// No longer care about alignment.
		tz::memblk ret{.ptr = h, .size = count};
		if(this->cursor + compulsive_size > this->arena.size_bytes())
		{
			return tz::nullblk;
		}
		this->cursor += compulsive_size;
		return ret;
	}

	void linear_allocator::deallocate(tz::memblk blk)
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

	bool linear_allocator::owns(tz::memblk blk) const
	{
		return std::greater_equal<const void*>{}(blk.ptr, this->arena.data()) && std::less_equal<const void*>{}(blk.ptr, this->arena.data() + this->arena.size_bytes());
	}

	std::byte* linear_allocator::head()
	{
		return this->arena.data() + this->cursor;
	}
}
