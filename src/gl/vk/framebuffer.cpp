#if TZ_VULKAN
#include "gl/vk/framebuffer.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    Framebuffer::Framebuffer(const RenderPass& render_pass, const ImageView& image_view, VkExtent2D dimensions):
    frame_buffer(VK_NULL_HANDLE),
    device(&render_pass.get_device())
    {
        auto img_view_native = image_view.native();
        VkFramebufferCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create.renderPass = render_pass.native();
        create.attachmentCount = 1;
        create.pAttachments = &img_view_native;
        create.width = dimensions.width;
        create.height = dimensions.height;
        create.layers = 1;

        auto res = vkCreateFramebuffer(this->device->native(), &create, nullptr, &this->frame_buffer);
        tz_assert(res == VK_SUCCESS, "Failed to create Framebuffer");
    }

    Framebuffer::Framebuffer(Framebuffer&& move):
    frame_buffer(VK_NULL_HANDLE),
    device(nullptr)
    {
        *this = std::move(move);
    }

    Framebuffer::~Framebuffer()
    {
        if(this->frame_buffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(this->device->native(), this->frame_buffer, nullptr);
            this->frame_buffer = VK_NULL_HANDLE;
        }
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& rhs)
    {
        std::swap(this->frame_buffer, rhs.frame_buffer);
        std::swap(this->device, rhs.device);
        return *this;
    }
    
    VkFramebuffer Framebuffer::native() const
    {
        return this->frame_buffer;
    }

}

#endif // TZ_VULKAN