#ifndef TOPAZ_GL_VK_SETUP_SWAPCHAIN_HPP
#define TOPAZ_GL_VK_SETUP_SWAPCHAIN_HPP
#if TZ_VULKAN
#include "gl/vk/impl/setup/logical_device.hpp"
#include "gl/vk/impl/hardware/swapchain_selector.hpp"
#include "gl/vk/impl/setup/image_view.hpp"
#include <vector>
#include <span>

namespace tz::gl::vk
{
    class Swapchain
    {
    public:
        Swapchain(const LogicalDevice& device, VkSurfaceKHR surface, hardware::SwapchainSelectorPreferences preferences = hardware::default_swapchain_preferences);
        Swapchain(const LogicalDevice& device, hardware::SwapchainSelectorPreferences preferences = hardware::default_swapchain_preferences);
        Swapchain(const Swapchain& copy) = delete;
        Swapchain(Swapchain&& move);
        ~Swapchain();

        Swapchain& operator=(const Swapchain& rhs) = delete;
        Swapchain& operator=(Swapchain&& rhs);
        float get_width() const;
        float get_height() const;
        std::span<const ImageView> get_image_views() const;

        VkExtent2D native_extent() const;
        VkFormat native_format() const;
    private:
        VkSwapchainKHR swapchain;
        const LogicalDevice* logical_device;
        std::vector<VkImage> images;
        std::vector<ImageView> image_views;
        VkFormat format;
        VkExtent2D extent;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SETUP_SWAPCHAIN_HPP