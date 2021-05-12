#if TZ_OGL
#include "core/assert.hpp"
#include "gl/ogl/setup/opengl_instance.hpp"
#include "GLFW/glfw3.h"
#include "glad/glad.h"

namespace tz::gl::ogl
{
    OpenGLInstance::OpenGLInstance(tz::GameInfo game_info)
    {
        [[maybe_unused]] int glad_load_result = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
        tz_assert(glad_load_result != 0, "gladLoadGLLoader returned error");
    }

    OpenGLInstance::~OpenGLInstance()
    {
        
    }
}

#endif // TZ_OGL