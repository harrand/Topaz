#ifndef TOPAZ_GL_IMPL_VULKAN_BACKEND_DRAW_HPP
#define TOPAZ_GL_IMPL_VULKAN_BACKEND_DRAW_HPP
#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"

namespace tz::gl::vk2
{
	struct draw_indirect_command
	{
		std::uint32_t count;
		std::uint32_t instance_count = 1;
		std::uint32_t first;
		std::uint32_t base_instance = 0;
	};
	struct draw_indexed_indirect_command
	{
		std::uint32_t count;
		std::uint32_t instance_count = 1;
		std::uint32_t first_index;
		std::int32_t base_vertex = 0;
		std::uint32_t base_instance = 0;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_VULKAN_BACKEND_DRAW_HPP