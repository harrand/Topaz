#ifndef TOPAZ_GL_API_DEVICE_HPP
#define TOPAZ_GL_API_DEVICE_HPP
#include "gl/renderer.hpp"
#include "gl/shader.hpp"

#include "gl/impl/frontend/common/device.hpp"

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

    /**
     * @brief Top-level interface through which `tz::gl` mechanisms can be constructed. Devices can be described as follows:
     * - A device can spawn any number of `tz::gl` mechanisms, such as render-passes, renderers or shaders. It has sole ownership of anything it spawns.
     * - Ownership transfer of these mechanisms to another Device is not supported.
     * - Any mechanisms spawned by the device are valid until the device reaches the end of its lifetime.
     */
    class IDevice
    {
    public:
        /**
         * @brief Create an @ref IRenderer using this device and the provided builder.
         * @note Renderer inputs are copied over, so the renderer input provided is no longer needed once the renderer has been created.
         * 
         * @param builder Builder describing the parameters of the renderer.
         * @return The created renderer.
         */
        [[nodiscard]] virtual Renderer create_renderer(RendererBuilder builder) const = 0;
        /**
         * @brief Create an @ref IShader using this device and the provided builder.
         *
         * @param builder Builder describing the sources and modules of the shader program.
         * @return Shader Shader program which can be provided to a `tz::gl::Renderer`.
         */
        [[nodiscard]] virtual Shader create_shader(ShaderBuilder builder) const = 0;
        /**
         * @brief Retrieve the texture format used by the window. Note that in headless applications, the return value is implementation-defined.
         * 
         * @return TextureFormat Format corresponding to the window framebuffer.
         */
        virtual TextureFormat get_window_format() const = 0;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_DEVICE_HPP