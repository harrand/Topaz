#ifndef TOPAZ_CORE_MEMORY_HPP
#define TOPAZ_CORE_MEMORY_HPP

namespace tz
{
	/**
	 * @ingroup tz_core
	 * Represents an arbitrary, non-owning block of memory.
	 */
	struct Blk
	{
		/// Pointer to the start of the memory block.
		void* ptr;
		/// Size of the block, in bytes.
		std::size_t size;

		bool operator==(const Blk& rhs) const = default;
	};
	constexpr Blk nullblk{.ptr = nullptr, .size = 0};
}

#endif // TOPAZ_CORE_MEMORY_HPP
