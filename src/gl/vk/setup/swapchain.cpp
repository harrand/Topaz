#if TZ_VULKAN
#include "gl/vk/setup/swapchain.hpp"
#include "gl/vk/tz_vulkan.hpp"

namespace tz::gl::vk
{
    Swapchain::Swapchain(const LogicalDevice& device, VkSurfaceKHR surface, hardware::SwapchainSelectorPreferences preferences):
    swapchain(VK_NULL_HANDLE),
    logical_device(&device)
    {
        hardware::DeviceQueueFamily queue_family = device.get_queue_family();

        tz_assert(queue_family.dev != nullptr, "tz::gl::vk::Swapchain::Swapchain(...): Queue Family is not associated to any physical device!");
        hardware::SwapchainSupportDetails support = queue_family.dev->get_window_swapchain_support();
        auto maybe_format = hardware::select_swapchain_format(support.formats, preferences.format_pref);
        auto maybe_present_mode = hardware::select_swapchain_present_mode(support.present_modes, preferences.present_mode_pref);
        auto extent = hardware::get_real_swap_extent(support.capabilities);
        
        tz_assert(maybe_format.has_value(), "tz::gl::vk::Swapchain::Swapchain(...): No valid format found");
        tz_assert(maybe_present_mode.has_value(), "tz::gl::vk::Swapchain::Swapchain(...): No valid present mode found");
        
        std::uint32_t image_count = support.capabilities.maxImageCount + 1; // We'll go with at least one more than the mimimum as a hardcoded default.
        image_count = std::min(image_count, support.capabilities.maxImageCount); // This shouldn't really be a problem but lets be safe.

        VkSwapchainCreateInfoKHR create{};
        create.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create.surface = surface;
        create.minImageCount = image_count;
        create.imageFormat = maybe_format->format;
        create.imageColorSpace = maybe_format->colorSpace;
        create.imageExtent = extent;
        create.imageArrayLayers = 1;
        // TODO: Don't hardcode this so we can render into a separate image first (post-processing?)
        create.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // TODO: Don't assume we use the same queue family for graphics and present
        // It looks like most of the time we will be exclusive and it offers better perf. Is it reasonable to require it to be this way and don't handle the concurrent case?
        tz_assert(queue_family.types_supported.contains(hardware::QueueFamilyTypeField{std::initializer_list<hardware::QueueFamilyType>{hardware::QueueFamilyType::Graphics, hardware::QueueFamilyType::Present}}), "tz::gl::vk::Swapchain::Swapchain(...): Provided a queue family which doesn't support both graphics and present. Concurrent sharing mode is not yet implemented -- Only exclusive");
        create.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create.queueFamilyIndexCount = 0;
        create.pQueueFamilyIndices = nullptr;

        create.preTransform = support.capabilities.currentTransform;
        // TODO: Suport alpha blending with other windows in the window system.
        // No support for that right now, but maybe in the future?
        create.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        create.presentMode = maybe_present_mode.value();
        create.clipped = tz::gl::vk::is_headless() ? VK_FALSE : VK_TRUE; // We don't care about color of obscured pixels (e.g if another window covers them). Obviously if there is no window then this is irrelevant and we don't bother.

        // TODO: Support this. We will need this for example if we want the window to be able to be resized.
        create.oldSwapchain = VK_NULL_HANDLE;

        auto res = vkCreateSwapchainKHR(device.native(), &create, nullptr, &this->swapchain);
        tz_assert(res == VK_SUCCESS, "Failed to create swapchain");
    }

    Swapchain::Swapchain(const LogicalDevice& device, hardware::SwapchainSelectorPreferences preferences):
    Swapchain(device, vk::window_surface()->native(), preferences){}

    Swapchain::Swapchain(Swapchain&& move):
    swapchain(VK_NULL_HANDLE),
    logical_device(nullptr)
    {
        std::swap(this->swapchain, move.swapchain);
        std::swap(this->logical_device, move.logical_device);
    }

    Swapchain::~Swapchain()
    {
        if(this->swapchain != VK_NULL_HANDLE)
        {
            tz_assert(this->logical_device != nullptr, "tz::gl::vk::Swapchain::~Swapchain(): Logical device was null");
            vkDestroySwapchainKHR(this->logical_device->native(), this->swapchain, nullptr);
        }
    }

    Swapchain& Swapchain::operator=(Swapchain&& rhs)
    {
        std::swap(this->swapchain, rhs.swapchain);
        std::swap(this->logical_device, rhs.logical_device);
        return *this;
    }

}

#endif // TZ_VULKAN