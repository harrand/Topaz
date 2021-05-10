#ifndef TOPAZ_GL_IMPL_COMMON_RENDER_PASS_ATTACHMENT_HPP
#define TOPAZ_GL_IMPL_COMMON_RENDER_PASS_ATTACHMENT_HPP

namespace tz::gl
{
    enum class RenderPassAttachment
    {
        Colour,
        Depth,
        ColourDepth,
    };
}

#endif // TOPAZ_GL_IMPL_COMMON_RENDER_PASS_ATTACHMENT_HPP