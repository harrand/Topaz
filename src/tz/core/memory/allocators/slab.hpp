#ifndef TOPAZ_CORE_ALLOCATORS_SLAB_HPP
#define TOPAZ_CORE_ALLOCATORS_SLAB_HPP
#include "tz/core/memory/memblk.hpp"
#include "tz/core/memory/allocators/linear.hpp"
#include <memory>
#include <cstddef>
#include <vector>

namespace tz
{
	/**
	 * @ingroup tz_core_memory_allocator
	 * An allocator which creates large slabs of implementation-defined memory just-in-time, and continually suballocates from said slabs.
	 */
	class slab_allocator
	{
	public:
		slab_allocator() = default;

		tz::memblk allocate(std::size_t sz);
		void deallocate(tz::memblk blk);
		bool owns(tz::memblk blk) const;
	private:
		struct slab_t
		{
			slab_t(std::size_t sz);

			slab_t(const slab_t& cpy);
			slab_t(slab_t&& mv) = default;
			~slab_t() = default;
			slab_t& operator=(const slab_t& cpy);
			slab_t& operator=(slab_t&& mv) = default;

			std::unique_ptr<char[]> mem = nullptr;
			tz::linear_allocator alloc = {};
			std::size_t sz;
		};

		slab_t& new_slab(std::size_t sz);

		std::vector<slab_t> slabs = {};
	};
}

#endif // TOPAZ_CORE_ALLOCATORS_SLAB_HPP