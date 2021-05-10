#ifndef TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP

#include "gl/impl/common/render_pass_attachment.hpp"
#include "gl/vk/render_pass.hpp"

namespace tz::gl
{
    class RenderPassBuilderVulkan
    {
    public:
        RenderPassBuilderVulkan() = default;
        void add_pass(RenderPassAttachment attachment);
        void finalise();

        friend class RenderPassVulkan;
    private:
        vk::RenderPassBuilder builder;
    };

    class RenderPassVulkan
    {
    public:
        RenderPassVulkan(const vk::LogicalDevice& device, RenderPassBuilderVulkan builder);
    private:
        vk::RenderPass render_pass;
    };
}

#endif // TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP