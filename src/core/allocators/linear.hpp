#ifndef TOPAZ_CORE_ALLOCATOR_LINEAR_HPP
#define TOPAZ_CORE_ALLOCATOR_LINEAR_HPP
#include <span>
#include <cstddef>

namespace tz
{
	/**
	 * @ingroup tz_core_allocators
	 * An allocator with a known capacity. Allocator is provided a pre-allocated memory block on construction. Allocations retrieve sections of the memory block, deallocations are no-ops.
	 */
	template<typename T>
	class LinearAllocator
	{
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using propagatate_on_container_move_assignment = std::true_type;

		/**
		 * Create an allocator which manages a block of memory via the arena span.
		 */
		LinearAllocator(std::span<std::byte> arena);
		/**
		 * Attempt to allocate memory from the arena. If there is not enough space for the allocation, an assert will fail, but the function may ultimately return nullptr.
		 */
		T* allocate(std::size_t count);
		/**
		 * Deallocate existing memory. For linear allocators this does absolutely nothing.
		 */
		void deallocate(T* addr, std::size_t count){}
	private:
		std::byte* get_head() const;
		std::size_t get_bytes_remaining() const;

		std::span<std::byte> arena;
		std::size_t offset = 0;
	};

	/**
	 * Functions just like a @ref LinearAllocator, except that a scratch allocator comes with its own pre-allocated chunk of memory which is used as the arena for the linear allocator. The arena is therefore guaranteed to be on the stack, ensuring excellent performance at the cost of memory safety.
	 * @tparam T Type of object which will be allocated.
	 * @tparam S Number of T's which can be stored within the pre-allocated arena.
	 */
	template<typename T, std::size_t S>
	class ScratchLinearAllocator : public LinearAllocator<T>
	{
	public:
		template<class U>
		struct rebind
		{
			using other = ScratchLinearAllocator<U, S>;
		};

		/**
		 * Create a new scratch linear allocator.
		 * @note Due to the memory arena being local to the constructing stack-frame, anything using this allocator must not perform any allocations which outlive the allocator itself.
		 */
		ScratchLinearAllocator();
	private:
		std::aligned_storage<sizeof(T), alignof(T)> data[S];
	};
}
#include "core/allocators/linear.inl"

#endif // TOPAZ_CORE_ALLOCATOR_LINEAR_HPP
