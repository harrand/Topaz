#ifndef TOPAZ_GL_IMPL_OGL_DEVICE_HPP
#define TOPAZ_GL_IMPL_OGL_DEVICE_HPP
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"

namespace tz::gl
{
    class DeviceOGL
    {
    public:
        DeviceOGL() = default;
        RenderPass create_render_pass(RenderPassBuilder builder) const;
        Renderer create_renderer(RendererBuilder builder) const;
    private:

    };
}

#endif // TOPAZ_GL_IMPL_OGL_DEVICE_HPP