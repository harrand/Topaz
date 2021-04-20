#if TZ_VULKAN
#include "gl/vk/framebuffer.hpp"

namespace tz::gl::vk
{
    Framebuffer::Framebuffer(const pipeline::RenderPass& render_pass, const Swapchain& swapchain, std::size_t swapchain_image_view_index):
    framebuffer(VK_NULL_HANDLE),
    device(&render_pass.get_device())
    {
        VkFramebufferCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create.renderPass = render_pass.native();

        VkImageView swapchain_view = swapchain.get_image_views()[swapchain_image_view_index].native();

        create.attachmentCount = 1;
        create.pAttachments = &swapchain_view;
        create.width = swapchain.get_width();
        create.height = swapchain.get_height();
        create.layers = 1;

        auto res = vkCreateFramebuffer(this->device->native(), &create, nullptr, &this->framebuffer);
    }

    Framebuffer::Framebuffer(Framebuffer&& move):
    framebuffer(VK_NULL_HANDLE),
    device(nullptr)
    {
        *this = std::move(move);
    }

    Framebuffer::~Framebuffer()
    {
        if(this->framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(this->device->native(), this->framebuffer, nullptr);
        }
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& rhs)
    {
        std::swap(this->framebuffer, rhs.framebuffer);
        std::swap(this->device, rhs.device);
        return *this;
    }

    VkFramebuffer Framebuffer::native() const
    {
        return this->framebuffer;
    }

    FramebufferList::FramebufferList(const pipeline::RenderPass& render_pass, const Swapchain& swapchain)
    {
        for(std::size_t i = 0; i < swapchain.get_image_views().size(); i++)
        {
            this->emplace(render_pass, swapchain, i);
        }
    }
}

#endif // TZ_VULKAN