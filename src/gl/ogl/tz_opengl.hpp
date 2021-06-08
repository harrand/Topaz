#ifndef TOPAZ_GL_OGL_TZ_OPENGL_HPP
#define TOPAZ_GL_OGL_TZ_OPENGL_HPP
#if TZ_OGL
#include "gl/ogl/setup/opengl_instance.hpp"

namespace tz::gl::ogl
{
    void initialise_headless(tz::GameInfo game_info);
    void initialise(tz::GameInfo game_info);
    void terminate();
    OpenGLInstance& get();
    bool is_initialised();

    constexpr static tz::Version get_opengl_version()
    {
        return {4, 6, 0};
    }
}

#endif // TZ_OGL
#endif // TOPAZ_GL_OGL_TZ_OPENGL_HPP