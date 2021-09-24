#if TZ_VULKAN
#include "gl/impl/backend/vk/swapchain.hpp"
#include "gl/impl/backend/vk/tz_vulkan.hpp"
#include <limits>

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

        tz_assert(queue_family.dev != nullptr, "tz::gl::vk::Swapchain::Swapchain(...): Queue Family is not associated with any physical device!");
        hardware::SwapchainSupportDetails support = queue_family.dev->get_window_swapchain_support();
        auto maybe_format = hardware::select_swapchain_format(support.formats, preferences.format_pref);
        auto maybe_present_mode = hardware::select_swapchain_present_mode(support.present_modes, preferences.present_mode_pref);
        auto extent = hardware::get_real_swap_extent(support.capabilities);
        
        tz_assert(maybe_format.has_value(), "tz::gl::vk::Swapchain::Swapchain(...): No valid format found");
        tz_assert(maybe_present_mode.has_value(), "tz::gl::vk::Swapchain::Swapchain(...): No valid present mode found");
        
        std::uint32_t image_count = support.capabilities.minImageCount; // We'll go with at least one more than the mimimum as a hardcoded default.
        image_count = std::min(image_count, support.capabilities.maxImageCount); // This shouldn't really be a problem but lets be safe.

        VkSwapchainCreateInfoKHR create{};
        create.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create.surface = surface;
        create.minImageCount = image_count;
        create.imageFormat = maybe_format->format;
        create.imageColorSpace = maybe_format->colorSpace;
        create.imageExtent = extent;
        create.imageArrayLayers = 1;
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

        this->format = maybe_format->format;
        this->extent = extent;

        std::vector<VkImage> img_handles = this->get_swapchain_image_natives();
        for(VkImage native : img_handles)
        {
            const vk::Image& img = this->images.emplace_back(*this->logical_device, native, static_cast<std::uint32_t>(this->get_width()), static_cast<std::uint32_t>(this->get_height()), this->get_format(), vk::Image::UsageField{vk::Image::Usage::ColourAttachment});
            this->image_views.emplace_back(*this->logical_device, img);
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

    Swapchain Swapchain::null()
    {
        return {};
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

    VkSwapchainKHR Swapchain::native() const
    {
        return this->swapchain;
    }


    VkRect2D Swapchain::full_render_area() const
    {
        VkRect2D rect;
        rect.extent = this->native_extent();
        rect.offset = {0, 0};
        return rect;
    }

    Image::Format Swapchain::get_format() const
    {
        return static_cast<Image::Format>(this->format);
    }

    Swapchain::AcquireResult Swapchain::acquire_next_image_index(const Semaphore& semaphore) const
    {
        std::uint32_t image_index;
        auto res = vkAcquireNextImageKHR(this->logical_device->native(), this->swapchain, std::numeric_limits<std::uint64_t>::max(), semaphore.native(), VK_NULL_HANDLE, &image_index);
        switch(res)
        {
            case VK_SUCCESS:
                return {image_index, Swapchain::AcquireResponseType::Fine};
            break;
            case VK_ERROR_OUT_OF_DATE_KHR:
                return {std::nullopt, Swapchain::AcquireResponseType::ErrorSwapchainOutOfDate};
            break;
            default:
                return {std::nullopt, Swapchain::AcquireResponseType::ErrorUnknown};
            break;
        }
    }

    const LogicalDevice& Swapchain::get_device() const
    {
        return *this->logical_device;
    }

    std::vector<VkImage> Swapchain::get_swapchain_image_natives() const
    {
        std::vector<VkImage> image_natives;
        std::uint32_t image_count;
        vkGetSwapchainImagesKHR(this->logical_device->native(), this->swapchain, &image_count, nullptr);
        image_natives.resize(image_count);
        vkGetSwapchainImagesKHR(this->logical_device->native(), this->swapchain, &image_count, image_natives.data());
        return image_natives;
    }

    Swapchain::Swapchain():
    swapchain(VK_NULL_HANDLE),
    logical_device(nullptr),
    images(),
    image_views(),
    format(VK_FORMAT_UNDEFINED),
    extent(){}
}

#endif // TZ_VULKAN