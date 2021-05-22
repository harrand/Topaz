#ifndef TOPAZ_GL_API_DEVICE_HPP
#define TOPAZ_GL_API_DEVICE_HPP
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"
#include "gl/shader.hpp"

#include "gl/impl/common/device.hpp"

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */
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
        /**
         * @brief Create a renderer using this device and the provided builder.
         * @note Renderer inputs are copied over, so the renderer input provided is no longer needed once the renderer has been created.
         * 
         * @param builder Describes the resultant Renderer.
         * @return The created renderer.
         */
        virtual Renderer create_renderer(RendererBuilder builder) const = 0;
        virtual Shader create_shader(ShaderBuilder builder) const = 0;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_DEVICE_HPP