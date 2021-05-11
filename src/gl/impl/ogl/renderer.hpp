#ifndef TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#define TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#include "gl/api/renderer.hpp"

namespace tz::gl
{
    class RendererBuilderOGL : public IRendererBuilder
    {

    };

    class RendererOGL : public IRenderer
    {
    public:
        RendererOGL(RendererBuilderOGL builder);
    private:
        
    };
}

#endif // TOPAZ_GL_IMPL_OGL_RENDERER_HPP