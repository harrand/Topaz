#ifndef TOPAZ_GL_IMPL_OGL_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_OGL_RENDER_PASS_HPP
#if TZ_OGL

#include "gl/api/render_pass.hpp"
#include <vector>
#include <span>

namespace tz::gl
{
    class RenderPassBuilderOGL : public IRenderPassBuilder
    {
    public:
        RenderPassBuilderOGL() = default;
        virtual void add_pass(RenderPassAttachment attachment) final;
        std::span<const RenderPassAttachment> ogl_get_attachments() const;
    private:
        std::vector<RenderPassAttachment> attachments;
    };

    class RenderPassOGL : public IRenderPass
    {
    public:
        RenderPassOGL(RenderPassBuilderOGL builder);
        virtual bool requires_depth_image() const final;
        std::span<const RenderPassAttachment> ogl_get_attachments() const;
    private:
        RenderPassBuilderOGL subpasses;
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_OGL_RENDER_PASS_HPP