#ifndef TOPAZ_GL_RENDER_PASS_HPP
#define TOPAZ_GL_RENDER_PASS_HPP

#if TZ_VULKAN
#include "gl/vk/render_pass.hpp"
#elif TZ_OGL
#include <vector>
#endif

namespace tz::gl
{
    enum class RenderPassAttachment
    {
        Colour,
        Depth,
        ColourDepth,
    };

#if TZ_VULKAN
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

    using RenderPassBuilder = RenderPassBuilderVulkan;
    using RenderPass = RenderPassVulkan;
#elif TZ_OGL
    class RenderPassBuilderOGL
    {
    public:
        RenderPassBuilderOGL() = default;
        void add_pass(RenderPassAttachment attachment);
        void finalise(){}

        friend class RenderPassOGL;
    private:
        std::vector<RenderPassAttachment> attachments;
    };

    class RenderPassOGL
    {
    public:
        RenderPassOGL(RenderPassBuilderOGL builder);
    private:
        RenderPassBuilderOGL subpasses;
    };

    using RenderPassBuilder = RenderPassBuilderOGL;
    using RenderPass = RenderPassOGL;
#endif
}

#endif // TOPAZ_GL_RENDER_PASS_HPP