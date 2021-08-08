#ifndef TOPAZ_GL_IMPL_BACKEND_OGL_RENDERBUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL_RENDERBUFFER_HPP
#if TZ_OGL
#include "gl/impl/backend/ogl/texture.hpp"

namespace tz::gl::ogl
{
    class Renderbuffer
    {
    public:
        Renderbuffer(GLsizei width, GLsizei height, Texture::Format format);
        Renderbuffer(const Renderbuffer& copy) = delete;
        Renderbuffer(Renderbuffer&& move);
        ~Renderbuffer();

        Renderbuffer& operator=(const Renderbuffer& copy) = delete;
        Renderbuffer& operator=(Renderbuffer&& move);

        GLuint native() const;
    private:
        GLuint renderbuffer;
        GLsizei width;
        GLsizei height;
        Texture::Format format;
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL_RENDERBUFFER_HPP