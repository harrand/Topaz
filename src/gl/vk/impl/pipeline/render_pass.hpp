#ifndef TOPAZ_GL_VK_PIPELINE_RENDER_PASS_HPP
#define TOPAZ_GL_VK_PIPELINE_RENDER_PASS_HPP
#if TZ_VULKAN
#include "gl/vk/impl/pipeline/framebuffer_attachments.hpp"
#include "gl/vk/impl/setup/logical_device.hpp"

namespace tz::gl::vk::pipeline
{
    class RenderPass
    {
    public:
        RenderPass(const LogicalDevice& device, std::initializer_list<FramebufferAttachmentList> subpasses);
        RenderPass(const RenderPass& copy) = delete;
        RenderPass(RenderPass& move);
        ~RenderPass();

        RenderPass& operator=(const RenderPass& rhs) = delete;
        RenderPass& operator=(RenderPass&& rhs);

        VkRenderPass native() const;
        const LogicalDevice& get_device() const;
    private:
        const LogicalDevice* device;
        std::vector<FramebufferAttachmentList> subpasses;
        VkRenderPass render_pass;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_RENDER_PASS_HPP