#ifndef TOPAZ_GL_IMPL_OGL_DEVICE_HPP
#define TOPAZ_GL_IMPL_OGL_DEVICE_HPP
#if TZ_OGL
#include "gl/api/device.hpp"

namespace tz::gl
{
    class DeviceBuilderOGL : public IDeviceBuilder
    {
    public:
        DeviceBuilderOGL();
        virtual void set_primitive_type(GraphicsPrimitiveType type) final;
        virtual GraphicsPrimitiveType get_primitive_type() const final;

        friend class DeviceOGL;
    private:
        GraphicsPrimitiveType primitive_type;
    };

    class DeviceOGL : public IDevice
    {
    public:
        DeviceOGL(DeviceBuilderOGL builder);
        [[nodiscard]] virtual Renderer create_renderer(RendererBuilder builder) const final;
        [[nodiscard]] virtual Shader create_shader(ShaderBuilder builder) const final;
        virtual TextureFormat get_window_format() const final;
    private:
        void on_window_resize();

        GraphicsPrimitiveType primitive_type;
        mutable std::deque<DeviceWindowResizeCallback> renderer_resize_callbacks;
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_OGL_DEVICE_HPP