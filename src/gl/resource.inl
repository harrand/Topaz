#include "core/types.hpp"
#include "core/assert.hpp"
#include <cstring>
#include <memory>

namespace tz::gl2
{
	template<tz::TriviallyCopyable T>
	BufferResource BufferResource::from_one(const T& data)
	{
		return BufferResource::from_many<T>({data});
	}

	template<tz::TriviallyCopyable T>
	BufferResource BufferResource::from_many(std::span<const T> data)
	{
		std::vector<std::byte> resource_data;
		std::size_t space = alignof(T) + (sizeof(T) * data.size());
		std::size_t space_copy = space;
		resource_data.resize(space);
		void* data_ptr = resource_data.data();
		bool success = std::align(alignof(T), sizeof(T), data_ptr, space) != nullptr;
		tz_assert(success, "Storage for a single T (sizeof T + alignof T) was too small! Please submit a bug report.");

		std::memcpy(data_ptr, data.data(), sizeof(T) * data.size());
		tz_assert(space_copy >= space, "Alignment of pointer somehow increased space, this is a logic error. Please submit a bug report.");
		std::size_t alignment_usage = space_copy - space;
		// space is now reduced by `alignment_usage` bytes, meaning that when we resize the vector to `space` bytes it is guaranteed to be less than or equal to its initial size. C++ spec says that "Vector capacity is never reduced when resizing to a smaller size because that would invalidate all iterators..." meaning that no realloc took place and the memcpy'd object is still aligned properly.
		resource_data.resize(space);
		return {resource_data, alignment_usage};
	}
}
