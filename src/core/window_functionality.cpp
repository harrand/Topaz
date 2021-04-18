#include "core/window_functionality.hpp"
#include "core/assert.hpp"

namespace tz
{
    WindowFunctionality::WindowFunctionality(GLFWwindow* wnd):
    wnd(wnd){}

    GLFWwindow* WindowFunctionality::get_middleware_handle() const
    {
        return this->wnd;
    }

    bool WindowFunctionality::is_close_requested() const
    {
        this->ensure();
        return glfwWindowShouldClose(this->wnd) == GLFW_TRUE;
    }

    void WindowFunctionality::update()
    {
        glfwPollEvents();
        #if TZ_OPENGL
            // OpenGL only
            glfwSwapBuffers();
        #endif
    }

    void WindowFunctionality::ensure() const
    {
        tz_assert(this->wnd != nullptr, "WindowFunctionality::ensure(): Failed");
    }
}