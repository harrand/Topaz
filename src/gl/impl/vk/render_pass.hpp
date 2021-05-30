#ifndef TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP

#include "gl/api/render_pass.hpp"

#include "gl/vk/render_pass.hpp"
#include "gl/vk/swapchain.hpp"

namespace tz::gl
{
    class RenderPassBuilderVulkan : public IRenderPassBuilder
    {
    public:
        RenderPassBuilderVulkan() = default;
        virtual void add_pass(RenderPassAttachment attachment) final;
        void vk_finalise(vk::Image::Format colour_attachment_format);

        friend class RenderPassVulkan;
    private:
        vk::RenderPassBuilder builder;
        std::vector<RenderPassAttachment> passes;
    };

    struct RenderPassBuilderDeviceInfoVulkan
    {
        const vk::LogicalDevice* device;
        const vk::Swapchain* device_swapchain;
    };

    class RenderPassVulkan : public IRenderPass
    {
    public:
        RenderPassVulkan(RenderPassBuilderVulkan builder, RenderPassBuilderDeviceInfoVulkan device_info);
        virtual bool requires_depth_image() const final;
        const vk::RenderPass& vk_get_render_pass() const;
    private:
        vk::RenderPass render_pass;
        vk::Image::Format colour_attachment_format;
        bool has_depth_attachment;
    };
}

#endif // TOPAZ_GL_IMPL_VK_RENDER_PASS_HPP