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
    /**
     * Represents an interface used to describe the fundamental parameters of a @ref tz::gl::IDevice.
     */
    class IDeviceBuilder
    {
    public:
        /**
         * @brief Set the primitive type, which refers to how the renderer interprets input data.
         * 
         * @param type The primitive type we will be rendering with. The optimal type is triangles on all platforms.
         */
        virtual void set_primitive_type(GraphicsPrimitiveType type) = 0;
        /**
         * @brief Retrieve the primitive type, which refers to how the renderer interprets input data.
         * @note Unless you have set the primitive type manually, the default primitive type will always be `GraphicsPrimitiveType::Triangles`.
         * @return GraphicsPrimitiveType Primitive type used by this Device. 
         */
        virtual GraphicsPrimitiveType get_primitive_type() const = 0;
    };

    class IDevice
    {
    public:
        /**
         * @brief Create a render-pass using this device and the provided builder.
         * 
         * @param builder Builder describing the composition of the render-pass.
         * @return RenderPass 
         */
        virtual RenderPass create_render_pass(RenderPassBuilder builder) const = 0;
        /**
         * @brief Create a renderer using this device and the provided builder.
         * @note Renderer inputs are copied over, so the renderer input provided is no longer needed once the renderer has been created.
         * 
         * @param builder Builder describing the parameters of the renderer.
         * @return The created renderer.
         */
        virtual Renderer create_renderer(RendererBuilder builder) const = 0;
        /**
         * @brief Create a shader program using this device and the provided builder.
         *
         * @param builder Builder describing the sources and modules of the shader program.
         * @return Shader Shader program which can be provided to a `tz::gl::Renderer`.
         */
        virtual Shader create_shader(ShaderBuilder builder) const = 0;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_DEVICE_HPP