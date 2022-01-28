#include "core/types.hpp"
#include "core/assert.hpp"
#include <cstring>
#include <memory>

namespace tz::gl2
{
	template<tz::TriviallyCopyable T>
	BufferResource BufferResource::from_one(const T& data, ResourceAccess access)
	{
		return BufferResource::from_many<T>(std::span<const T>{&data, 1}, access);
	}

	template<std::ranges::contiguous_range R>
	BufferResource BufferResource::from_many(R&& data, ResourceAccess access)
	{
		using T = decltype(*std::ranges::begin(data));
		auto size = std::distance(std::ranges::begin(data), std::ranges::end(data));
		std::vector<std::byte> resource_data;
		std::size_t space = alignof(T) + (sizeof(T) * size);
		std::size_t space_copy = space;
		resource_data.resize(space);
		void* data_ptr = resource_data.data();
		bool success = std::align(alignof(T), sizeof(T), data_ptr, space) != nullptr;
		tz_assert(success, "Storage for a single T (sizeof T + alignof T) was too small! Please submit a bug report.");

		std::memcpy(data_ptr, std::ranges::data(data), sizeof(T) * data.size());
		tz_assert(space_copy >= space, "Alignment of pointer somehow increased space, this is a logic error. Please submit a bug report.");
		std::size_t alignment_usage = space_copy - space;
		// space is now reduced by `alignment_usage` bytes, meaning that when we resize the vector to `space` bytes it is guaranteed to be less than or equal to its initial size. C++ spec says that "Vector capacity is never reduced when resizing to a smaller size because that would invalidate all iterators..." meaning that no realloc took place and the memcpy'd object is still aligned properly.
		resource_data.resize(space);
		return {access, resource_data, alignment_usage};
	}

	ImageResource ImageResource::from_memory(ImageFormat format, tz::Vec2ui dimensions, std::ranges::contiguous_range auto data, ResourceAccess access)
	{
		using T = std::decay_t<decltype(*std::ranges::begin(data))>;
		auto size = std::distance(std::ranges::begin(data), std::ranges::end(data));
		std::span<const std::byte> byte_data = std::as_bytes(std::span<const T>(std::ranges::data(data), size));
		std::size_t pixel_size = tz::gl2::pixel_size_bytes(format);
		std::vector<std::byte> resource_data(pixel_size * dimensions[0] * dimensions[1]);
		std::copy(byte_data.begin(), byte_data.end(), resource_data.begin());
		// TODO: Sanity check? Is it correct to just not give a shit about alignment here?
		return {access, resource_data, 0, format, dimensions};
	}

}
