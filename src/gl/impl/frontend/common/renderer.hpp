#ifndef TOPAZ_GL_IMPL_COMMON_RENDERER_HPP
#define TOPAZ_GL_IMPL_COMMON_RENDERER_HPP
#include "core/handle.hpp"

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */

    /**
     * @brief Describes how often the renderer elements should expect new attribute values.
     * 
     */
    enum class RendererInputFrequency
    {
        /// Input attributes appear on a per-vertex basis.
        PerVertexBasis,
        /// Input attributes appear on a per-instance basis.
        PerInstanceBasis
    };

    /**
     * @brief Describes the component type of a given input attribute.
     * 
     */
    enum class RendererComponentType
    {
        /// One 32-bit float, aka a vec1.
        Float32,
        /// Two 32-bit floats, aka a vec2.
        Float32x2,
        /// Three 32-bit floats, aka a vec3.
        Float32x3
    };

    /**
     * @brief Describes culling strategy of a Renderer. Renderers natively support culling of faces. See @ref IRendererBuilder for defaults.
     */
    enum class RendererCullingStrategy
    {
        /// No faces are culled.
        NoCulling,
        /// Front-faces are culled.
        CullFrontFaces,
        /// Back-faces are culled.
        CullBackFaces,
        /// Everything is culled.
        CullEverything
    };

    /**
     * @brief Describes how IRendererInput data can be accessed after Renderer construction.
     */
    enum class RendererInputDataAccess
    {
        /// Renderer input data is fixed and immutable -- It cannot be resized or edited.
        StaticFixed,
        /// Renderer input data is fixed and mutable -- It cannot be resized but can be edited.
        DynamicFixed
    };

    enum class RendererOutputType
    {
        Window,
        Texture
    };
    /**
     * @} 
     */
}

#endif // TOPAZ_GL_IMPL_COMMON_RENDERER_HPP