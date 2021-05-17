#ifndef TOPAZ_GL_IMPL_OGL_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_OGL_RENDER_PASS_HPP

#include "gl/api/render_pass.hpp"
#include <vector>

namespace tz::gl
{
    class RenderPassBuilderOGL : public IRenderPassBuilder
    {
    public:
        RenderPassBuilderOGL() = default;
        virtual void add_pass(RenderPassAttachment attachment) final;

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