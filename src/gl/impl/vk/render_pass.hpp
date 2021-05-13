#ifndef TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP

#include "gl/api/render_pass.hpp"
#include "gl/vk/render_pass.hpp"

namespace tz::gl
{
    class RenderPassBuilderVulkan : public IRenderPassBuilder
    {
    public:
        RenderPassBuilderVulkan() = default;
        virtual void add_pass(RenderPassAttachment attachment) final;
        virtual void finalise() final;

        friend class RenderPassVulkan;
    private:
        vk::RenderPassBuilder builder;
    };

    class RenderPassVulkan
    {
    public:
        RenderPassVulkan(const vk::LogicalDevice& device, RenderPassBuilderVulkan builder);
        const vk::RenderPass& vk_get_render_pass() const;
    private:
        vk::RenderPass render_pass;
    };
}

#endif // TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP