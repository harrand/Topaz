#ifndef TOPAZ_CORE_WINDOW_HPP
#define TOPAZ_CORE_WINDOW_HPP
#include "core/containers/basic_list.hpp"
#include "core/window_functionality.hpp"

namespace tz
{
    struct WindowHint
    {
        int hint;
        int value;
    };

    struct WindowInitArgs
    {
        int width;
        int height;
        const char* title;
    };

    constexpr WindowInitArgs default_args = {.width = 800, .height = 600, .title = "Untitled"};

    using WindowHintList = tz::BasicList<WindowHint>;
    
    class Window : public WindowFunctionality
    {
    public:
        Window(WindowInitArgs args = default_args, WindowHintList hints = {});
        Window(const Window& copy) = delete;
        Window(Window&& move);
        ~Window();
        Window& operator=(const Window& rhs) = delete;
        Window& operator=(Window&& rhs);

        static void window_resize_callback(GLFWwindow* window, int width, int height);
    };
}

#endif // TOPAZ_CORE_WINDOW_HPP