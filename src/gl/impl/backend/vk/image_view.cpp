#if TZ_VULKAN

#include "gl/impl/backend/vk/image_view.hpp"
#include "gl/impl/backend/vk/swapchain.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    ImageView::ImageView(const LogicalDevice& device, const Image& image):
    view(VK_NULL_HANDLE),
    image(&image),
    device(&device)
    {
        VkImageViewCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        create.image = image.native();
        create.viewType = VK_IMAGE_VIEW_TYPE_2D;

        create.format = static_cast<VkFormat>(image.get_format());
        create.components.a = create.components.r = create.components.g = create.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
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
    image(nullptr),
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
        std::swap(this->image, rhs.image);
        std::swap(this->device, rhs.device);
        return *this;
    }   

    const Image* ImageView::get_image() const
    {
        return this->image;
    }

    VkImageView ImageView::native() const
    {
        return this->view;
    }
}

#endif // TZ_VULKAN