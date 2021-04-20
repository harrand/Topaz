#ifndef TOPAZ_GL_VK_IMAGE_HPP
#define TOPAZ_GL_VK_IMAGE_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"

namespace tz::gl::vk
{
    class Image
    {
    public:
        enum class Format
        {
            Undefined = VK_FORMAT_UNDEFINED,
            Rgba32Signed = VK_FORMAT_R8G8B8A8_SINT,
            Rgba32Unsigned = VK_FORMAT_R8G8B8A8_UINT,
            Rgba32sRGB = VK_FORMAT_R8G8B8A8_SRGB,
        };
        enum class Layout
        {
            Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
            ColourAttachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            DepthAttachment = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        };
    private:
    };
}

#endif
#endif // TOPAZ_GL_VK_IMAGE_HPP