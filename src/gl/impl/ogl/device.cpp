#if TZ_OGL
#include "gl/impl/ogl/device.hpp"

namespace tz::gl
{
    DeviceBuilderOGL::DeviceBuilderOGL():
    primitive_type(GraphicsPrimitiveType::Triangles)
    {

    }

    void DeviceBuilderOGL::set_primitive_type(GraphicsPrimitiveType type)
    {
        this->primitive_type = type;
    }

    GraphicsPrimitiveType DeviceBuilderOGL::get_primitive_type() const
    {
        return this->primitive_type;
    }

    DeviceOGL::DeviceOGL(DeviceBuilderOGL builder):
    primitive_type(builder.get_primitive_type())
    {
        
    }

    RenderPass DeviceOGL::create_render_pass(RenderPassBuilder builder) const
    {
        return {builder};
    }

    Renderer DeviceOGL::create_renderer(RendererBuilder builder) const
    {
        return {builder};
    }

    Shader DeviceOGL::create_shader(ShaderBuilder builder) const
    {
        return {};
    }
}
#endif // TZ_OGL