#ifndef TOPAZ_GL_API_DEVICE_HPP
#define TOPAZ_GL_API_DEVICE_HPP
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"
#include "gl/shader.hpp"

#include "gl/impl/common/device.hpp"

namespace tz::gl
{
    class IDeviceBuilder
    {
    public:
        /**
         * @brief Set the primitive type for all rendering done by this Device.
         * 
         * @param type The primitive type we will be rendering with. The optimal type is triangles on all platforms.
         */
        virtual void set_primitive_type(GraphicsPrimitiveType type) = 0;
        /**
         * @brief Get the primitive type used for rendering done by this Device.
         * 
         * @return GraphicsPrimitiveType Primitive type used by this Device.
         */
        virtual GraphicsPrimitiveType get_primitive_type() const = 0;
    };

    class IDevice
    {
    public:
        virtual RenderPass create_render_pass(RenderPassBuilder builder) const = 0;
        virtual Renderer create_renderer(RendererBuilder builder) const = 0;
        virtual Shader create_shader(ShaderBuilder builder) const = 0;
    };
}

#endif // TOPAZ_GL_API_DEVICE_HPP