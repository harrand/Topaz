#ifndef TOPAZ_GL_RENDERER_HPP
#define TOPAZ_GL_RENDERER_HPP

#if TZ_VULKAN
#include "gl/impl/vk/renderer.hpp"
namespace tz::gl
{
    using RendererBuilder = RendererBuilderVulkan;
    using Renderer = RendererVulkan;
}
#elif TZ_OGL
#include "gl/impl/ogl/renderer.hpp"
namespace tz::gl
{
    using RendererBuilder = RendererBuilderOGL;
    using Renderer = RendererOGL;
}
#endif

#endif // TOPAZ_GL_RENDERER_HPP