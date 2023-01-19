#ifndef TZ_MEMORY_MEMBLK_HPP
#define TZ_MEMORY_MEMBLK_HPP
#include <cstddef>

namespace tz
{
	/**
	* @ingroup tz
	* A non-owning, contiguous block of memory.
	*/
	struct memblk
	{
		/// Start address of the block.
		void* ptr;
		/// Size of the block, in bytes.
		std::size_t size;
		
		bool operator==(const memblk& rhs) const = default;
	};
	/// Represents the null block.
	constexpr memblk nullblk{.ptr = nullptr, .size = 0};
}

#endif // TZ_MEMORY_MEMBLK_HPP
