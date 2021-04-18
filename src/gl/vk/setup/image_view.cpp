#if TZ_VULKAN

#include "gl/vk/setup/image_view.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
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
}

#endif // TZ_VULKAN