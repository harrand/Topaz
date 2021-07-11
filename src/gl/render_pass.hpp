#ifndef TOPAZ_GL_RENDER_PASS_HPP
#define TOPAZ_GL_RENDER_PASS_HPP

#if TZ_VULKAN
#include "gl/impl/frontend/vk/render_pass.hpp"
namespace tz::gl
{
    using RenderPassBuilder = RenderPassBuilderVulkan;
    using RenderPass = RenderPassVulkan;
}
#elif TZ_OGL
#include "gl/impl/frontend/ogl/render_pass.hpp"
namespace tz::gl
{
    using RenderPassBuilder = RenderPassBuilderOGL;
    using RenderPass = RenderPassOGL;
}
#endif

#endif // TOPAZ_GL_RENDER_PASS_HPP