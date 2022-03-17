#ifndef TOPAZ_CORE_ALLOCATORS_ADAPTER_HPP
#define TOPAZ_CORE_ALLOCATORS_ADAPTER_HPP
#include "core/types.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_allocators
	 * A meta-allocator which allows a topaz allocator to be used as if it were a `std::allocator`. This means it can be used for standard-library containers.
	 *
	 * For instance:
	 * `std::vector<int, std::allocator<int>>` is functionally equivalent to `std::vector<int, tz::AllocatorAdapter<int, tz::Mallocator>>`
	 */
	template<typename T, tz::Allocator A>
	class AllocatorAdapter : private A
	{
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using propagate_on_container_move_assignment = std::true_type;

		AllocatorAdapter() = default;

		template<class U>
		AllocatorAdapter(const AllocatorAdapter<U, A>& other){}

		T* allocate(std::size_t n)
		{
			return reinterpret_cast<T*>(A::allocate(n * sizeof(T)).ptr);
		}

		void deallocate(T* p, std::size_t n)
		{
			A::deallocate({.ptr = p, .size = n});
		}
	};
}

#endif // TOPAZ_CORE_ALLOCATORS_ADAPTER_HPP
