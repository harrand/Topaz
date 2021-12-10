#ifndef TOPAZ_CORE_ALLOCATORS_LINEAR_HPP
#define TOPAZ_CORE_ALLOCATORS_LINEAR_HPP
#include <span>
#include <vector>
#include <cstddef>

namespace tz
{
	namespace detail
	{
		struct BoolProxy
		{
			bool val;
			bool operator==(const BoolProxy& rhs) const = default;
		};
	}
	/**
	 * @ingroup tz_core
	 */
	template<typename T>
	class LinearAllocator
	{
	public:
		using pointer = T*;
		using const_pointer = const T*;
		using void_pointer = void*;
		using const_void_pointer = const void*;
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		constexpr LinearAllocator(void* ptr, std::size_t size_bytes);
		constexpr LinearAllocator(std::span<T> elements);
		constexpr LinearAllocator(const LinearAllocator<T>& copy) noexcept;
		constexpr LinearAllocator(LinearAllocator<T>&& move) noexcept;
		template<typename B, typename = std::enable_if_t<!std::is_same_v<T, B>>>
		constexpr LinearAllocator(const LinearAllocator<B>& separate_copy) noexcept;
		template<typename B, typename = std::enable_if_t<!std::is_same_v<T, B>>>
		constexpr LinearAllocator(LinearAllocator<B>&& separate_move) noexcept;
		~LinearAllocator() = default;
		LinearAllocator& operator=(const LinearAllocator<T>& rhs) noexcept;
		LinearAllocator& operator=(LinearAllocator<T>&& rhs) noexcept;

		pointer allocate(size_type n) noexcept;
		// pointer allocate(size_type n, const_void_pointer cvp) noexcept; // not implemented.
		void deallocate(pointer ptr, size_type n) noexcept;

		size_type max_size() const;
		size_type size() const;
		size_type size_bytes() const;

		pointer data() const;
		bool exists_at(std::size_t idx) const;

		bool operator==(const LinearAllocator<T>& rhs) const noexcept;
		bool operator!=(const LinearAllocator<T>& rhs) const noexcept;
	private:
		void clear();
		void delete_at(std::size_t idx);

		std::span<std::byte> bytes;
		std::vector<detail::BoolProxy> object_mask;
	};
}
#include "core/allocators/linear.inl"

#endif // TOPAZ_CORE_ALLOCATORS_LINEAR_HPP
