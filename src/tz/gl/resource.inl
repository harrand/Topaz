#include "tz/core/types.hpp"
#include "tz/core/debug.hpp"
#include <cstring>
#include <memory>

namespace tz::gl
{
	template<tz::trivially_copyable T>
	buffer_resource buffer_resource::from_one(const T& data, buffer_info info)
	{
		return buffer_resource::from_many<T>({data}, info);
	}

	template<std::ranges::contiguous_range R>
	buffer_resource buffer_resource::from_many(R&& data, buffer_info info)
	{
		using T = decltype(*std::ranges::begin(data));
		auto size = std::distance(std::ranges::begin(data), std::ranges::end(data));
		std::vector<std::byte> resource_data;
		std::size_t space = (sizeof(T) * size);
		std::size_t space_copy = space;
		resource_data.resize(space);
		void* data_ptr = resource_data.data();
		bool success = std::align(alignof(T), sizeof(T), data_ptr, space) != nullptr;
		tz::assert(success, "Storage for a single T (sizeof T + alignof T) was too small! Please submit a bug report.");

		std::memcpy(data_ptr, std::ranges::data(data), sizeof(T) * data.size());
		tz::assert(space_copy >= space, "Alignment of pointer somehow increased space, this is a logic error. Please submit a bug report.");
		std::size_t alignment_usage = space_copy - space;
		// space is now reduced by `alignment_usage` bytes, meaning that when we resize the vector to `space` bytes it is guaranteed to be less than or equal to its initial size. C++ spec says that "Vector capacity is never reduced when resizing to a smaller size because that would invalidate all iterators..." meaning that no realloc took place and the memcpy'd object is still aligned properly.
		resource_data.resize(space);
		return {info.access, resource_data, alignment_usage, info.flags};
	}

	image_resource image_resource::from_memory(std::ranges::contiguous_range auto data, image_info info)
	{
		using T = std::decay_t<decltype(*std::ranges::begin(data))>;
		auto size = std::distance(std::ranges::begin(data), std::ranges::end(data));
		std::span<const std::byte> byte_data = std::as_bytes(std::span<const T>(std::ranges::data(data), size));
		std::size_t pixel_size = tz::gl::pixel_size_bytes(info.format);
		std::vector<std::byte> resource_data(pixel_size * info.dimensions[0] * info.dimensions[1]);
		std::copy(byte_data.begin(), byte_data.end(), resource_data.begin());
		// TODO: Sanity check? Is it correct to just not give a shit about alignment here?
		return {info.access, resource_data, 0, info.format, info.dimensions, info.flags};
	}

}
