#ifndef TOPAZ_CORE_ALLOCATORS_ADAPTER_HPP
#define TOPAZ_CORE_ALLOCATORS_ADAPTER_HPP
#include "tz/core/memory/memblk.hpp"
#include "tz/core/types.hpp"
#include "tz/core/debug.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_memory_allocator
	 * A meta-allocator which allows a tz allocator to be used as if it were a `std::allocator`. This means it can be used for standard-library containers.
	 *
	 * For instance:
	 * `std::vector<int, std::allocator<int>>` is functionally equivalent to `std::vector<int, tz::allocator_adapter<int, tz::Mallocator>>`
	 */
	template<typename T, tz::allocator A>
	class allocator_adapter : private A
	{
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using propagate_on_container_move_assignment = std::true_type;

		allocator_adapter() = default;
		allocator_adapter(const A& a):
		A(a){}

		template<class U>
		allocator_adapter(const allocator_adapter<U, A>& other){}

		T* allocate(std::size_t n)
		{
			tz::memblk blk = A::allocate(n * sizeof(T));
			tz::assert(blk != nullblk, "allocator_adapter -- The underlying tz allocator returned a null-block. Memory allocation failed. The standard library allocator is going to crash.");
			return reinterpret_cast<T*>(blk.ptr);
		}

		void deallocate(T* p, std::size_t n)
		{
			A::deallocate({.ptr = p, .size = n});
		}
	};
}

#endif // TOPAZ_CORE_ALLOCATORS_ADAPTER_HPP
