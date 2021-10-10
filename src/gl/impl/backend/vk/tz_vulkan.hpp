#ifndef TOPAZ_GL_VK_TZ_VULKAN_HPP
#define TOPAZ_GL_VK_TZ_VULKAN_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/setup/vulkan_instance.hpp"
#include "gl/impl/backend/vk/setup/window_surface.hpp"

namespace tz::gl::vk
{
	void initialise_headless(tz::GameInfo game_info);
	void initialise(tz::GameInfo game_info);
	void terminate();
	VulkanInstance& get();
	bool is_initialised();
	WindowSurface* window_surface();
	bool is_headless();

	constexpr VkPhysicalDeviceFeatures required_rendering_features()
	{
		VkPhysicalDeviceFeatures features{};
		features.multiDrawIndirect = VK_TRUE;
		return features;
	}

	constexpr static tz::Version get_vulkan_version()
	{
		return {1, 1, 175};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_TZ_VULKAN_HPP