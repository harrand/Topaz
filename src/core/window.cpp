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
        #endif

        this->wnd = glfwCreateWindow(args.width, args.height, args.title, nullptr, nullptr);
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
}