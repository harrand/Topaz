#if TZ_OGL
#include "core/tz.hpp"
#include "gl/impl/frontend/ogl/device.hpp"

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
    primitive_type(builder.get_primitive_type()),
    renderer_resize_callbacks()
    {
        // Setup window resize support.
        tz::window().add_resize_callback([this](int width, int height)
        {
            int w = width;
            int h = height;
            while(w == 0 || h == 0)
            {
                w = tz::window().get_width();
                h = tz::window().get_height();
                tz::Window::block_until_event_happens();
            }
            this->on_window_resize();
        });
    }

    Renderer DeviceOGL::create_renderer(RendererBuilder builder) const
    {
        RendererDeviceInfoOGL device_info;
        device_info.on_resize = &this->renderer_resize_callbacks.emplace_back(nullptr);
        return {builder, device_info};
    }

    Shader DeviceOGL::create_shader(ShaderBuilder builder) const
    {
        return {builder};
    }

    Processor DeviceOGL::create_processor(ProcessorBuilder builder) const
    {
        return {builder};
    }

    TextureFormat DeviceOGL::get_window_format() const
    {
        return TextureFormat::Bgra32UnsignedNorm;
    }

    void DeviceOGL::on_window_resize()
    {
        // Notify all renderers
        for(auto i = this->renderer_resize_callbacks.rbegin(); i < this->renderer_resize_callbacks.rend(); i++)
        {
            const DeviceWindowResizeCallback& callback = *i;
            if(callback != nullptr)
            {
                callback();
            }
        }
    }
}
#endif // TZ_OGL