#ifndef TOPAZ_GL_OGL_SETUP_OPENGL_INSTANCE_HPP
#define TOPAZ_GL_OGL_SETUP_OPENGL_INSTANCE_HPP
#if TZ_OGL
#include "core/game_info.hpp"
#include "glad/glad.h"

namespace tz::gl::ogl
{
    class OpenGLInstance
    {
    public:
        static void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_data);

        OpenGLInstance(tz::GameInfo game_info);
        ~OpenGLInstance();
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_OGL_SETUP_OPENGL_INSTANCE_HPP