#ifndef TOPAZ_GL_VK_HARDWARE_SWAPCHAIN_SUPPORT_HPP
#define TOPAZ_GL_VK_HARDWARE_SWAPCHAIN_SUPPORT_HPP
#include "vulkan/vulkan.h"
#include "core/containers/basic_list.hpp"

namespace tz::gl::vk::hardware
{
    using SurfaceCapabilities = VkSurfaceCapabilitiesKHR;
    using SurfaceFormat = VkSurfaceFormatKHR;
    using SurfaceFormats = tz::BasicList<SurfaceFormat>;
    using SurfacePresentMode = VkPresentModeKHR;
    using SurfacePresentModes = tz::BasicList<SurfacePresentMode>;

    struct SwapchainSupportDetails
    {
        SurfaceCapabilities capabilities;
        SurfaceFormats formats;
        SurfacePresentModes present_modes;
        bool supports_swapchain;
    };
}

#endif // TOPAZ_GL_VK_HARDWARE_SWAPCHAIN_SUPPORT_HPP