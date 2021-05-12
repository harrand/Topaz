#ifndef TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#define TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#include "gl/api/renderer.hpp"
#include "glad/glad.h"
#include <optional>

namespace tz::gl
{
    class RendererBuilderOGL : public IRendererBuilder
    {
    public:
        RendererBuilderOGL() = default;
        virtual void set_element_format(RendererElementFormat element_format) final;
        virtual RendererElementFormat get_element_format() const final;
    private:
        std::optional<RendererElementFormat> format;
    };

    class RendererOGL : public IRenderer
    {
    public:
        RendererOGL(RendererBuilderOGL builder);
        RendererOGL(const RendererOGL& copy) = delete;
        RendererOGL(RendererOGL&& move);
        ~RendererOGL();

        RendererOGL& operator=(const RendererOGL& rhs) = delete;
        RendererOGL& operator=(RendererOGL&& rhs);
    private:
        GLuint vao;
    };
}

#endif // TOPAZ_GL_IMPL_OGL_RENDERER_HPP