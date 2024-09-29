#ifndef TOPAZ_GPU_RESOURCE_HPP
#define TOPAZ_GPU_RESOURCE_HPP
#include "tz/core/handle.hpp"
#include <span>

namespace tz::gpu
{
	enum class resource_access
	{
		static_access,
		dynamic_access
	};

	struct buffer_info
	{
		resource_access access;
		std::span<const std::byte> data;
		const char* debug_name = "";
	};

	struct image_info
	{
		resource_access access;
		unsigned int width;
		unsigned int height;
		std::span<const std::byte> data;
		const char* debug_name = "";
	};

	using resource_handle = tz::handle<buffer_info>;

	resource_handle create_buffer(buffer_info);
	resource_handle create_image(image_info);
}

#endif // TOPAZ_GPU_RESOURCE_HPP