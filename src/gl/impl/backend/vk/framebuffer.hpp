#ifndef TOPAZ_GL_VK_FRAMEBUFFER_HPP
#define TOPAZ_GL_VK_FRAMEBUFFER_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/render_pass.hpp"
#include "gl/impl/backend/vk/image_view.hpp"

namespace tz::gl::vk
{
    class Framebuffer
    {
    public:
        Framebuffer(const RenderPass& render_pass, const ImageView& col_view, VkExtent2D dimensions);
        Framebuffer(const RenderPass& render_pass, const ImageView& col_view, const ImageView& depth_view, VkExtent2D dimensions);
        Framebuffer(const Framebuffer& copy) = delete;
        Framebuffer(Framebuffer&& move);
        ~Framebuffer();

        Framebuffer& operator=(const Framebuffer& rhs) = delete;
        Framebuffer& operator=(Framebuffer&& rhs);
        const ImageView& get_colour_view() const;
        const ImageView* get_depth_view() const;
        VkFramebuffer native() const;

    private:
        VkFramebuffer frame_buffer;
        const ImageView* colour_view;
        const ImageView* depth_view;
        const LogicalDevice* device;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_FRAMEBUFFER_HPP