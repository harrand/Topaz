#ifndef TOPAZ_GL_VK_HARDWARE_SWAPCHAIN_SELECTOR_HPP
#define TOPAZ_GL_VK_HARDWARE_SWAPCHAIN_SELECTOR_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include <optional>
#include <span>
#include <vector>

namespace tz::gl::vk::hardware
{
	enum class SwapchainFormatPreferences
	{
		Goldilocks, // I will only accept the absolute perfect swapchain format
		FlexibleGoldilocks, // I will rank each swapchain format by how close it is to the perfect format, and choose the highest-ranked
		DontCare
	};

	std::optional<VkSurfaceFormatKHR> select_swapchain_format(std::span<VkSurfaceFormatKHR> formats, SwapchainFormatPreferences preferences);
	std::optional<VkSurfaceFormatKHR> select_swapchain_format(std::span<VkSurfaceFormatKHR> formats, std::span<SwapchainFormatPreferences> preferences);

	enum class SwapchainPresentModePreferences
	{
		PreferTripleBuffering,
		DontCare
	};

	std::optional<VkPresentModeKHR> select_swapchain_present_mode(std::span<VkPresentModeKHR> present_modes, SwapchainPresentModePreferences preferences);
	std::optional<VkPresentModeKHR> select_swapchain_present_mode(std::span<VkPresentModeKHR> present_modes, std::span<SwapchainPresentModePreferences> preferences);

	VkExtent2D get_real_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

	struct SwapchainSelectorPreferences
	{
		std::vector<SwapchainFormatPreferences> format_pref;
		std::vector<SwapchainPresentModePreferences> present_mode_pref;
	};

	// TODO: (C++20 compiler support for constexpr std::vector) -- Make this constexpr
	const SwapchainSelectorPreferences default_swapchain_preferences{{SwapchainFormatPreferences::DontCare}, {SwapchainPresentModePreferences::DontCare}};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_HARDWARE_SWAPCHAIN_SELECTOR_HPP