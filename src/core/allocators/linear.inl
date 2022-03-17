#include "core/report.hpp"
#include "core/assert.hpp"
#include <memory>

namespace tz
{
	template<typename T>
	LinearAllocator<T>::LinearAllocator(std::span<std::byte> arena):
	arena(arena){}

	template<typename T>
	template<typename U>
	LinearAllocator<T>::LinearAllocator(const LinearAllocator<U>& other):
	arena(const_cast<LinearAllocator<U>&>(other).get_arena()),
	offset(other.get_offset()){}

	template<typename T>
	T* LinearAllocator<T>::allocate(std::size_t count)
	{
		void* p = this->get_head();
		std::size_t space = this->get_bytes_remaining();
		p = std::align(alignof(T), sizeof(T) * count, p, space);
		
		if(p == nullptr)
		{
			tz_error("LinearAllocation failed due to running out of space. Allocation requested %zu bytes but only have %zu available", count * sizeof(T), space);
			return nullptr;
		}
		std::size_t space_used = (this->get_bytes_remaining() - space) + (sizeof(T) * count);
		this->offset += space_used;
		return static_cast<T*>(p);
	}

	template<typename T>
	std::span<std::byte> LinearAllocator<T>::get_arena()
	{
		return this->arena;
	}

	template<typename T>
	std::size_t LinearAllocator<T>::get_offset() const
	{
		return this->offset;
	}

	template<typename T>
	std::byte* LinearAllocator<T>::get_head() const
	{
		return this->arena.data() + offset;
	}

	template<typename T>
	std::size_t LinearAllocator<T>::get_bytes_remaining() const
	{
		return this->arena.size_bytes() - std::distance(this->arena.data(), this->get_head());
	}

	template<typename T, std::size_t S>
	ScratchLinearAllocator<T, S>::ScratchLinearAllocator():
	LinearAllocator<T>({reinterpret_cast<std::byte*>(this->data), sizeof(T) * S}){}
}
