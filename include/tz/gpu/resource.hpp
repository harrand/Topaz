#ifndef TOPAZ_GPU_RESOURCE_HPP
#define TOPAZ_GPU_RESOURCE_HPP
#include "tz/core/error.hpp"
#include "tz/core/handle.hpp"
#include <span>
#include <expected>

namespace tz::gpu
{
	enum class resource_access
	{
		static_access,
		dynamic_access
	};

	enum class buffer_type
	{
		storage,
		index
	};

	struct buffer_info
	{
		resource_access access;
		buffer_type type;
		std::span<const std::byte> data;
		const char* debug_name = "";
	};

	enum class image_type
	{
		rgba,
		depth,
		floats,
	};

	struct image_info
	{
		resource_access access;
		unsigned int width;
		unsigned int height;
		image_type type;
		std::span<const std::byte> data;
		const char* debug_name = "";
	};

	using resource_handle = tz::handle<buffer_info>;

	std::expected<resource_handle, tz::error_code> create_buffer(buffer_info);
	std::expected<resource_handle, tz::error_code> create_image(image_info);

	tz::error_code destroy_resource(resource_handle res);
}

#endif // TOPAZ_GPU_RESOURCE_HPP