#include "core/window_functionality.hpp"
#include "core/assert.hpp"

#if TZ_OGL
#include "glad/glad.h"
#endif

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

    float WindowFunctionality::get_width() const
    {
        return static_cast<float>(this->get_size().first);
    }

    float WindowFunctionality::get_height() const
    {
        return static_cast<float>(this->get_size().second);
    }

    void WindowFunctionality::update()
    {
        glfwPollEvents();
        #if TZ_OGL
            // OpenGL only
            glfwSwapBuffers(this->wnd);
        #endif
    }

    void WindowFunctionality::set_render_target() const
    {
        #if TZ_VULKAN

        #elif TZ_OGL
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, static_cast<GLsizei>(this->get_width()), static_cast<GLsizei>(this->get_height()));
        #endif
    }

    void WindowFunctionality::block_until_event_happens()
    {
        glfwWaitEvents();
    }

    std::pair<int, int> WindowFunctionality::get_size() const
    {
        int w, h;
        glfwGetFramebufferSize(this->wnd, &w, &h);
        return {w, h};
    }

    void WindowFunctionality::ensure() const
    {
        tz_assert(this->wnd != nullptr, "WindowFunctionality::ensure(): Failed");
    }
}