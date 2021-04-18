#if TZ_VULKAN
#include "gl/vk/hardware/swapchain_selector.hpp"
#include "core/tz.hpp"
#include <vector>
#include <algorithm>

namespace tz::gl::vk::hardware
{
    constexpr VkSurfaceFormatKHR perfect_format{.format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    
    constexpr bool is_perfect(VkSurfaceFormatKHR format)
    {
        return format.format == perfect_format.format && format.colorSpace == perfect_format.colorSpace;
    }

    constexpr int rate_perfect(VkSurfaceFormatKHR format)
    {
        // TODO: Implement
        return 0;
    }

    void sort_by_rate_perfect(std::span<VkSurfaceFormatKHR> formats)
    {
        std::sort(formats.begin(), formats.end(), [](VkSurfaceFormatKHR lhs, VkSurfaceFormatKHR rhs)
        {
            return rate_perfect(lhs) < rate_perfect(rhs);
        });
    }

    std::optional<VkSurfaceFormatKHR> select_swapchain_format(std::span<VkSurfaceFormatKHR> formats, SwapchainFormatPreferences preferences)
    {
        switch(preferences)
        {
            case SwapchainFormatPreferences::Goldilocks:
                for(VkSurfaceFormatKHR format : formats)
                {
                    if(is_perfect(format))
                    {
                        return format;
                    }
                }
                return std::nullopt;
            break;

            case SwapchainFormatPreferences::FlexibleGoldilocks:
                if(!formats.empty())
                {
                    sort_by_rate_perfect(formats);
                    return formats.front();
                }
                return std::nullopt;
            break;

            case SwapchainFormatPreferences::DontCare:
            {
                if(!formats.empty())
                {
                    return formats.front();
                }
                return std::nullopt;
            }
        }
    }

    std::optional<VkSurfaceFormatKHR> select_swapchain_format(std::span<VkSurfaceFormatKHR> formats, std::span<SwapchainFormatPreferences> preferences)
    {
        for(SwapchainFormatPreferences pref : preferences)
        {
            auto maybe_format = select_swapchain_format(formats, pref);
            if(maybe_format.has_value())
            {
                return maybe_format.value();
            }
        }
        return std::nullopt;
    }

    std::optional<VkPresentModeKHR> select_swapchain_present_mode(std::span<VkPresentModeKHR> present_modes, SwapchainPresentModePreferences preferences)
    {
        switch(preferences)
        {
            case SwapchainPresentModePreferences::PreferTripleBuffering:
                for(VkPresentModeKHR present_mode : present_modes)
                {
                    if(present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
                    {
                        // New queued images just replace the old. Can be used for triple buffering to prevent swap tearing
                        return present_mode;
                    }
                }
                return std::nullopt;
            break;
            case SwapchainPresentModePreferences::DontCare:
                if(!present_modes.empty())
                {
                    return present_modes.front();
                }
                return std::nullopt;
            break;
        }
    }

    std::optional<VkPresentModeKHR> select_swapchain_present_mode(std::span<VkPresentModeKHR> present_modes, std::span<SwapchainPresentModePreferences> preferences)
    {
        for(SwapchainPresentModePreferences pref : preferences)
        {
            auto maybe_present_mode = select_swapchain_present_mode(present_modes, pref);
            if(maybe_present_mode.has_value())
            {
                return maybe_present_mode.value();
            }
        }
        return std::nullopt;
    }

    VkExtent2D fix_to_extremes(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D extent)
    {
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return extent;
    }

    VkExtent2D get_real_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if(capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        int width, height;
        glfwGetFramebufferSize(tz::window().get_middleware_handle(), &width, &height);
        VkExtent2D actual_extent{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
        return fix_to_extremes(capabilities, actual_extent);
    }
}

#endif // TZ_VULKAN