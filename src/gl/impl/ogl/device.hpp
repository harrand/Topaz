#ifndef TOPAZ_GL_IMPL_OGL_DEVICE_HPP
#define TOPAZ_GL_IMPL_OGL_DEVICE_HPP
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"
#include "gl/impl/common/device.hpp"

namespace tz::gl
{
    class DeviceBuilderOGL
    {
    public:
        DeviceBuilderOGL();
        void set_primitive_type(GraphicsPrimitiveType type);
        GraphicsPrimitiveType get_primitive_type() const;

        friend class DeviceOGL;
    private:
        GraphicsPrimitiveType primitive_type;
    };

    class DeviceOGL
    {
    public:
        DeviceOGL(DeviceBuilderOGL builder);
        RenderPass create_render_pass(RenderPassBuilder builder) const;
        Renderer create_renderer(RendererBuilder builder) const;
    private:
        GraphicsPrimitiveType primitive_type;
    };
}

#endif // TOPAZ_GL_IMPL_OGL_DEVICE_HPP