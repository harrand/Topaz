#if TZ_VULKAN

#include "gl/impl/backend/vk/image_view.hpp"
#include "gl/impl/backend/vk/swapchain.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    ImageView::ImageView(const LogicalDevice& device, const Image& image):
    view(VK_NULL_HANDLE),
    device(&device)
    {
        VkImageViewCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        create.image = image.native();
        create.viewType = VK_IMAGE_VIEW_TYPE_2D;

        create.format = static_cast<VkFormat>(image.get_format());
        switch(image.get_format())
        {
            case Image::Format::DepthFloat32:
                create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
            default:
                create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            break;
        }
        create.subresourceRange.baseMipLevel = 0;
        create.subresourceRange.levelCount = 1;
        create.subresourceRange.baseArrayLayer = 0;
        create.subresourceRange.layerCount = 1;
        auto res = vkCreateImageView(this->device->native(), &create, nullptr, &this->view);
        tz_assert(res == VK_SUCCESS, "Failed to create image view");
    }

    ImageView::ImageView(ImageView&& move):
    view(VK_NULL_HANDLE),
    device(nullptr)
    {
        *this = std::move(move);
    }

    ImageView::~ImageView()
    {
        if(this->view != VK_NULL_HANDLE)
        {
            vkDestroyImageView(this->device->native(), this->view, nullptr);
            this->view = VK_NULL_HANDLE;
        }
    }

    ImageView& ImageView::operator=(ImageView&& rhs)
    {
        std::swap(this->view, rhs.view);
        std::swap(this->device, rhs.device);
        return *this;
    }   

    VkImageView ImageView::native() const
    {
        return this->view;
    }

    ImageView::ImageView(const LogicalDevice& device, VkImage image, VkFormat format):
    view(VK_NULL_HANDLE),
    device(&device)
    {
        VkImageViewCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create.image = image;

        // How data is interpreted
        create.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create.format = format;

        create.components.a = create.components.r = create.components.g = create.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create.subresourceRange.baseMipLevel = 0;
        create.subresourceRange.levelCount = 1;
        create.subresourceRange.baseArrayLayer = 0;
        create.subresourceRange.layerCount = 1;
        auto res = vkCreateImageView(this->device->native(), &create, nullptr, &this->view);
        tz_assert(res == VK_SUCCESS, "tz::gl::vk::ImageView::ImageView(...): Failed to create image view");
    }

    ImageViews::ImageViews(const Swapchain& swapchain)
    {
        VkSwapchainKHR swapchain_native = swapchain.native();
        std::vector<VkImage> image_natives;
        std::uint32_t image_count;
        vkGetSwapchainImagesKHR(swapchain.get_device().native(), swapchain_native, &image_count, nullptr);
        image_natives.resize(image_count);
        vkGetSwapchainImagesKHR(swapchain.get_device().native(), swapchain_native, &image_count, image_natives.data());
        for(auto image_native : image_natives)
        {
            ImageView view{swapchain.get_device(), image_native, static_cast<VkFormat>(swapchain.get_format())};
            this->add(std::move(view));
        }
    }

    ImageViews::ImageViews(std::span<ImageView> views):
    ImageViews()
    {
        for(ImageView& view : views)
        {
            this->add(std::move(view));
        }
    }

}

#endif // TZ_VULKAN