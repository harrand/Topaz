#ifndef TOPAZ_GL_VK_FRAMEBUFFER_HPP
#define TOPAZ_GL_VK_FRAMEBUFFER_HPP
#if TZ_VULKAN
#include "gl/vk/setup/swapchain.hpp"
#include "gl/vk/pipeline/render_pass.hpp"
#include "core/containers/basic_list.hpp"

namespace tz::gl::vk
{
    class Framebuffer
    {
    public:
        Framebuffer(const pipeline::RenderPass& render_pass, const Swapchain& swapchain, std::size_t swapchain_image_view_index);
        Framebuffer(const Framebuffer& copy) = delete;
        Framebuffer(Framebuffer&& move);
        ~Framebuffer();

        Framebuffer& operator=(const Framebuffer& rhs) = delete;
        Framebuffer& operator=(Framebuffer&& rhs);

        VkFramebuffer native() const;
    private:
        VkFramebuffer framebuffer;
        const LogicalDevice* device;
    };

    class FramebufferList : public tz::BasicList<Framebuffer>
    {
    public:
        FramebufferList(const pipeline::RenderPass& render_pass, const Swapchain& swapchain);
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_FRAMEBUFFER_HPP