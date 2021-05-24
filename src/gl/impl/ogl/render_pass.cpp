#if TZ_OGL
#include "core/assert.hpp"
#include "gl/impl/ogl/render_pass.hpp"

namespace tz::gl
{
    void RenderPassBuilderOGL::add_pass(RenderPassAttachment attachment)
    {
        tz_assert(this->attachments.empty(), "Multiple render passes is not yet implemented in OpenGL.");
        this->attachments.push_back(attachment);
    }

    std::span<const RenderPassAttachment> RenderPassBuilderOGL::ogl_get_attachments() const
    {
        return this->attachments;
    }

    RenderPassOGL::RenderPassOGL(RenderPassBuilderOGL builder):
    subpasses(builder)
    {}

    std::span<const RenderPassAttachment> RenderPassOGL::ogl_get_attachments() const
    {
        return this->subpasses.ogl_get_attachments();
    }
}
#endif // TZ_OGL