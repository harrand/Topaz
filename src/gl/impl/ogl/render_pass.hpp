#ifndef TOPAZ_GL_IMPL_OGL_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_OGL_RENDER_PASS_HPP

#include "gl/impl/common/render_pass_attachment.hpp"
#include <vector>

namespace tz::gl
{
    class RenderPassBuilderOGL
    {
    public:
        RenderPassBuilderOGL() = default;
        void add_pass(RenderPassAttachment attachment);
        void finalise(){}

        friend class RenderPassOGL;
    private:
        std::vector<RenderPassAttachment> attachments;
    };

    class RenderPassOGL
    {
    public:
        RenderPassOGL(RenderPassBuilderOGL builder);
    private:
        RenderPassBuilderOGL subpasses;
    };
}

#endif // TOPAZ_GL_IMPL_OGL_RENDER_PASS_HPP