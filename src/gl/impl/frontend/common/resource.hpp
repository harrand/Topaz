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
        DepthFloat32,
        Bgra32UnsignedNorm,

        Null
    };

    enum class TexturePropertyFilter
    {
        Nearest,
        Linear
    };

    enum class TextureAddressMode
    {
        ClampToEdge
    };

    struct TextureProperties
    {
        constexpr static TextureProperties get_default()
        {
            return
            {
                .min_filter = TexturePropertyFilter::Linear,
                .mag_filter = TexturePropertyFilter::Linear,

                .address_mode_u = TextureAddressMode::ClampToEdge,
                .address_mode_v = TextureAddressMode::ClampToEdge,
                .address_mode_w = TextureAddressMode::ClampToEdge
            };
        }

        TexturePropertyFilter min_filter;
        TexturePropertyFilter mag_filter;

        TextureAddressMode address_mode_u;
        TextureAddressMode address_mode_v;
        TextureAddressMode address_mode_w;
    };
    /**
     * @} 
     */
}
#endif // TOPAZ_GL_IMPL_COMMON_RESOURCE_HPP