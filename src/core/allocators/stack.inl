
namespace tz
{
	template<std::size_t S>
	tz::Blk StackAllocator<S>::allocate(std::size_t count)
	{
		char* h = this->head();
		std::size_t compulsive_size = std::max(count, alignof(std::max_align_t));
		// No longer care about alignment
		tz::Blk ret{.ptr = h, .size = count};
		if(this->offset + compulsive_size > S)
		{
			return tz::nullblk;
		}
		this->offset += compulsive_size;
		return ret;
	}

	template<std::size_t S>
	void StackAllocator<S>::deallocate(tz::Blk blk)
	{
		std::size_t compulsive_size = std::max(blk.size, alignof(std::max_align_t));
		if(this->offset < compulsive_size)
		{
			return;
		}
		auto after_blk = static_cast<char*>(blk.ptr);
		std::advance(after_blk, this->offset - compulsive_size);
		if(after_blk == this->head())
		{
			// This was the most recently allocated block, let's "deallocate" it.
			this->offset -= compulsive_size;
		}
		// Otherwise we literally do nothing.
	}

	template<std::size_t S>
	bool StackAllocator<S>::owns(tz::Blk blk) const
	{
		std::less_equal<const void*> le;
		std::greater_equal<const void*> ge;
		return ge(blk.ptr, this->data) && le(blk.ptr, this->data + S);
	}

	template<std::size_t S>
	char* StackAllocator<S>::head()
	{
		return this->data + this->offset;
	}
}
