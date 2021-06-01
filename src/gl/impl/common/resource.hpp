#ifndef TOPAZ_GL_IMPL_COMMON_RESOURCE_HPP
#define TOPAZ_GL_IMPL_COMMON_RESOURCE_HPP

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */
    enum class ResourceType
    {
        Buffer,
        Texture
    };

    enum class TextureFormat
    {
        Rgba32Signed,
        Rgba32Unsigned,
        Rgba32sRGB,
        DepthFloat32
    };
    /**
     * @} 
     */
}
#endif // TOPAZ_GL_IMPL_COMMON_RESOURCE_HPP