#ifndef TOPAZ_GL_VK_FRAMEBUFFER_HPP
#define TOPAZ_GL_VK_FRAMEBUFFER_HPP
#if TZ_VULKAN
#include "gl/vk/render_pass.hpp"
#include "gl/vk/impl/setup/image_view.hpp"

namespace tz::gl::vk
{
    class Framebuffer
    {
    public:
        Framebuffer(const RenderPass& render_pass, const ImageView& image_view, VkExtent2D dimensions);
        Framebuffer(const Framebuffer& copy) = delete;
        Framebuffer(Framebuffer&& move);
        ~Framebuffer();

        Framebuffer& operator=(const Framebuffer& rhs) = delete;
        Framebuffer& operator=(Framebuffer&& rhs);

    private:
        const LogicalDevice* device;
        VkFramebuffer frame_buffer;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_FRAMEBUFFER_HPP