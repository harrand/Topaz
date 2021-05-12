#if TZ_OGL
#include "gl/impl/ogl/renderer.hpp"

namespace tz::gl
{
    void RendererBuilderOGL::set_element_format(RendererElementFormat element_format)
    {
        this->format = {element_format};
    }

    RendererElementFormat RendererBuilderOGL::get_element_format() const
    {
        tz_assert(this->format.has_value(), "RendererBuilder has not had element format set yet");
        return this->format.value();
    }

    RendererOGL::RendererOGL(RendererBuilderOGL builder):
    vao(0)
    {
        tz_assert(builder.get_element_format().basis == RendererInputFrequency::PerVertexBasis, "Vertex data on a per-instance basis is not yet implemented");
        glGenVertexArrays(1, &this->vao);
    }

    RendererOGL::RendererOGL(RendererOGL&& move):
    vao(0)
    {
        *this = std::move(move);
    }

    RendererOGL::~RendererOGL()
    {
        if(this->vao != 0)
        {
            glDeleteVertexArrays(1, &this->vao);
        }
    }

    RendererOGL& RendererOGL::operator=(RendererOGL&& rhs)
    {
        std::swap(this->vao, rhs.vao);
        return *this;
    }
}


#endif // TZ_OGL