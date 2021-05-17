#include "core/window.hpp"
#include <utility>

namespace tz
{
    Window::Window(WindowInitArgs args, WindowHintList hints):
    WindowFunctionality(nullptr)
    {
        for(const WindowHint& hint : hints)
        {
            glfwWindowHint(hint.hint, hint.value);
        }

        #if TZ_VULKAN
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        #elif TZ_OGL
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            #if TZ_DEBUG
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            #else
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
            #endif
        #endif

        this->wnd = glfwCreateWindow(args.width, args.height, args.title, nullptr, nullptr);
        glfwMakeContextCurrent(this->wnd);
        #if TZ_OGL
            // Vulkan isn't fps-capped -- neither should opengl be.
            glfwSwapInterval(0);
        #endif
        glfwSetWindowUserPointer(this->wnd, this);
        glfwSetFramebufferSizeCallback(this->wnd, Window::window_resize_callback);
    }

    Window::Window(Window&& move):
    WindowFunctionality(nullptr)
    {
        std::swap(this->wnd, move.wnd);
    }

    Window::~Window()
    {
        if(this->wnd != nullptr)
        {
            glfwDestroyWindow(this->wnd);
            this->wnd = nullptr;
        }
    }

    Window& Window::operator=(Window&& rhs)
    {
        std::swap(this->wnd, rhs.wnd);
        return *this;
    }

    void Window::window_resize_callback(GLFWwindow* window, int width, int height)
    {
        WindowFunctionality* cur_window_func = reinterpret_cast<WindowFunctionality*>(glfwGetWindowUserPointer(window));
        Window* cur_window = static_cast<Window*>(cur_window_func);
        for(const auto& resize_callback : cur_window->window_resize_callbacks)
        {
            resize_callback(width, height);
        }
    }

}