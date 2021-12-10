#include <algorithm>
#include <cmath>
#include <memory>
#include <iterator>

namespace tz
{
	template<typename T>
	constexpr LinearAllocator<T>::LinearAllocator(void* ptr, std::size_t size_bytes):
	bytes(static_cast<std::byte*>(ptr), size_bytes),
	object_mask(size_bytes / sizeof(T), detail::BoolProxy{.val = false}){}

	template<typename T>
	constexpr LinearAllocator<T>::LinearAllocator(std::span<T> elements):
	LinearAllocator<T>(elements.data(), elements.size_bytes()){}

	template<typename T>
	constexpr LinearAllocator<T>::LinearAllocator(const LinearAllocator<T>& copy) noexcept:
	bytes(copy.bytes),
	object_mask(copy.object_mask){}

	template<typename T>
	constexpr LinearAllocator<T>::LinearAllocator(LinearAllocator<T>&& move) noexcept:
	bytes(move.bytes),
	object_mask(move.object_mask)
	{
		// Postcondition: a1 == a. In other words, move ctor cannot invalidate *this. So it's basically a copy...
	}

	template<typename T>
	template<typename B, typename>
	constexpr LinearAllocator<T>::LinearAllocator(const LinearAllocator<B>& separate_copy) noexcept:
	LinearAllocator<T>(separate_copy.data(), separate_copy.max_size())
	{
		for(std::size_t i = 0; i < separate_copy.size(); i++)
		{
			if(separate_copy.exists_at(i))
			{
				// the next sizeof(B) bytes are taken.
				// example: sizeof(T) == 4, sizeof(B) == 2
				// we separate_copy has max_size of 8, meaning byte capacity is 8*2 = 16 bytes
				// separate_copy.object_mask = {0, 1, 0, 0, 0, 0, 0, 0}
				// logic : this->object_mask = {1, 0, 0, 0}
				// num_taken_objects = ceil(2.0f / 4) == 1
				const size_type num_taken_objects = std::ceil(static_cast<float>(sizeof(B)) / sizeof(T));
				// object_ratio = ceil(4.0f / 2) == 2
				const size_type object_ratio = std::floor(static_cast<float>(sizeof(T)) / sizeof(B));
				// t_offset == 1 / 2 == 0
				const size_type t_offset = i / object_ratio;
				// 0th value
				auto iter = this->object_mask.begin() + t_offset;
				// 1st value
				auto end = iter + num_taken_objects;
				std::fill(iter, end, {.val = true});
			}
		}	
	}

	template<typename T>
	template<typename B, typename>
	constexpr LinearAllocator<T>::LinearAllocator(LinearAllocator<B>&& separate_move) noexcept:
	LinearAllocator<T>(separate_move){}

	template<typename T>
	LinearAllocator<T>& LinearAllocator<T>::operator=(const LinearAllocator<T>& rhs) noexcept
	{
		this->bytes = rhs.bytes;
		this->object_mask = rhs.object_mask;
		return *this;
	}

	template<typename T>
	LinearAllocator<T>& LinearAllocator<T>::operator=(LinearAllocator<T>&& rhs) noexcept
	{
		// Like move ctor, we cannot invalidate *this. It's just a copy at this point...
		this->bytes = rhs.bytes;
		this->object_mask = rhs.object_mask;
		return *this;
	}

	template<typename T>
	typename LinearAllocator<T>::pointer LinearAllocator<T>::allocate(LinearAllocator<T>::size_type n) noexcept
	{
		// Find the first free space(s).
		std::vector<detail::BoolProxy> chunks(n, detail::BoolProxy{.val = false});
		auto iter = std::search(this->object_mask.begin(), this->object_mask.end(), chunks.begin(), chunks.end());
		if(iter == this->object_mask.end())
		{
			// We are full, no more space. Return nullptr.
			return nullptr;
		}
		const size_type idx = std::distance(this->object_mask.begin(), iter);
		for(std::size_t i = 0; i < n; i++)
		{
			this->object_mask[idx + i] = {.val = true};
		}
		void* ret = this->bytes.data() + (sizeof(value_type) * idx);

		std::size_t space = this->max_size();
		if(std::align(alignof(T), sizeof(T), ret, space) != nullptr)
		{
			return reinterpret_cast<T*>(ret);
		}
		else
		{
			return nullptr;
		}
	}

	template<typename T>
	void LinearAllocator<T>::deallocate([[maybe_unused]] LinearAllocator<T>::pointer ptr, [[maybe_unused]] LinearAllocator<T>::size_type n) noexcept
	{
		size_type byte_dist = std::distance(this->bytes.data(), reinterpret_cast<std::byte*>(ptr));
		size_type idx = std::ceil(static_cast<float>(byte_dist) / sizeof(T));
		for(std::size_t i = 0; i < n; i++)
		{
			this->object_mask[idx + i] = {.val = false};
		}
	}

	template<typename T>
	typename LinearAllocator<T>::size_type LinearAllocator<T>::max_size() const
	{
		return this->bytes.size_bytes() / sizeof(value_type);
	}

	template<typename T>
	typename LinearAllocator<T>::size_type LinearAllocator<T>::size() const
	{
		return std::count_if(this->object_mask.begin(), this->object_mask.end(), [](const detail::BoolProxy& b){return b.val;});
	}

	template<typename T>
	typename LinearAllocator<T>::size_type LinearAllocator<T>::size_bytes() const
	{
		return this->size() * sizeof(value_type);
	}

	template<typename T>
	typename LinearAllocator<T>::pointer LinearAllocator<T>::data() const
	{
		return reinterpret_cast<pointer>(this->bytes.data());
	}

	template<typename T>
	bool LinearAllocator<T>::operator==(const LinearAllocator<T>& rhs) const noexcept
	{
		// Standard says: "Returns true only if storage allocated by a1 can be deallocated through a2."
		// Now because deallocation is a no-op, this is always true! Note that this doesn't mean we're a stateless allocator.
		return true;
	}

	template<typename T>
	bool LinearAllocator<T>::operator!=(const LinearAllocator<T>& rhs) const noexcept
	{
		// See operator== for justification.
		return false;
	}
	
	template<typename T>
	void LinearAllocator<T>::clear()
	{
		for(std::size_t i = 0; i < this->object_mask.size(); i++)
		{
			this->delete_at(i);
		}
		#if TZ_DEBUG
			// In debug mode we'll memzero too so bugs become more obvious.
			// Use std::fill because its constexpr
			std::fill(this->bytes.begin(), this->bytes.end(), std::byte{0});
		#endif // TZ_DEBUG
	}

	template<typename T>
	void LinearAllocator<T>::delete_at(std::size_t idx)
	{
		tz_assert(idx < this->size(), "Index out of range. %zu >= this->size() (%zu). Please submit a bug report.", idx, this->size());
		// If we're TriviallyDestructible, don't bother clearing anything.
		if constexpr(!std::is_trivially_destructible_v<value_type>)
		{
			// If we have an object here and we're not TriviallyDestructible, we must invoke the dtor.
			if(this->object_mask[idx].val)
			{
				reinterpret_cast<T*>(this->bytes[sizeof(value_type) * idx]->~T());
			}
		}
		this->object_mask[idx] = {.val = false};
	}
}
