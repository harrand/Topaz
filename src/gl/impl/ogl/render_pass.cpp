#if TZ_OGL
#include "gl/impl/ogl/render_pass.hpp"

namespace tz::gl
{
    void RenderPassBuilderOGL::add_pass(RenderPassAttachment attachment)
    {
        this->attachments.push_back(attachment);
    }

    RenderPassOGL::RenderPassOGL(RenderPassBuilderOGL builder):
    subpasses(builder)
    {}
}
#endif // TZ_OGL