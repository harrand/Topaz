#ifndef TOPAZ_GL_OGL_SETUP_OPENGL_INSTANCE_HPP
#define TOPAZ_GL_OGL_SETUP_OPENGL_INSTANCE_HPP
#if TZ_OGL
#include "core/game_info.hpp"

namespace tz::gl::ogl
{
    class OpenGLInstance
    {
    public:
        OpenGLInstance(tz::GameInfo game_info);
        ~OpenGLInstance();
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_OGL_SETUP_OPENGL_INSTANCE_HPP