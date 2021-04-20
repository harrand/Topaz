#if TZ_VULKAN
#include "gl/vk/impl/setup/swapchain.hpp"
#include "gl/vk/tz_vulkan.hpp"

namespace tz::gl::vk
{
    Swapchain::Swapchain(const LogicalDevice& device, VkSurfaceKHR surface, hardware::SwapchainSelectorPreferences preferences):
    swapchain(VK_NULL_HANDLE),
    logical_device(&device),
    images(),
    image_views(),
    format(),
    extent()
    {
        hardware::DeviceQueueFamily queue_family = device.get_queue_family();

        tz_assert(queue_family.dev != nullptr, "tz::gl::vk::Swapchain::Swapchain(...): Queue Family is not associated to any physical device!");
        hardware::SwapchainSupportDetails support = queue_family.dev->get_window_swapchain_support();
        auto maybe_format = hardware::select_swapchain_format(support.formats, preferences.format_pref);
        auto maybe_present_mode = hardware::select_swapchain_present_mode(support.present_modes, preferences.present_mode_pref);
        auto extent = hardware::get_real_swap_extent(support.capabilities);
        
        tz_assert(maybe_format.has_value(), "tz::gl::vk::Swapchain::Swapchain(...): No valid format found");
        tz_assert(maybe_present_mode.has_value(), "tz::gl::vk::Swapchain::Swapchain(...): No valid present mode found");
        
        std::uint32_t image_count = support.capabilities.minImageCount + 1; // We'll go with at least one more than the mimimum as a hardcoded default.
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

        // Fill data members
        vkGetSwapchainImagesKHR(device.native(), this->swapchain, &image_count, nullptr);
        this->images.resize(image_count);
        this->image_views.reserve(image_count);
        vkGetSwapchainImagesKHR(device.native(), this->swapchain, &image_count, this->images.data());

        this->format = maybe_format->format;
        this->extent = extent;

        for(const VkImage& img : this->images)
        {
            this->image_views.emplace_back(device, img, this->format);
        }
    }

    Swapchain::Swapchain(const LogicalDevice& device, hardware::SwapchainSelectorPreferences preferences):
    Swapchain(device, vk::window_surface()->native(), preferences){}

    Swapchain::Swapchain(Swapchain&& move):
    swapchain(VK_NULL_HANDLE),
    logical_device(nullptr),
    images(),
    image_views(),
    format(),
    extent()
    {
        *this = std::move(move);
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
        std::swap(this->images, rhs.images);
        std::swap(this->image_views, rhs.image_views);
        std::swap(this->format, rhs.format);
        std::swap(this->extent, rhs.extent);
        return *this;
    }

    float Swapchain::get_width() const
    {
        return this->extent.width;
    }

    float Swapchain::get_height() const
    {
        return this->extent.height;
    }

    std::span<const ImageView> Swapchain::get_image_views() const
    {
        return this->image_views;
    }

    VkExtent2D Swapchain::native_extent() const
    {
        return this->extent;
    }

    VkFormat Swapchain::native_format() const
    {
        return this->format;
    }

    Image::Format Swapchain::get_format() const
    {
        return static_cast<Image::Format>(this->format);
    }

}

#endif // TZ_VULKAN