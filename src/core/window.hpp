#ifndef TOPAZ_CORE_WINDOW_HPP
#define TOPAZ_CORE_WINDOW_HPP
#include "core/containers/basic_list.hpp"
#include "core/window_functionality.hpp"

namespace tz
{
    /**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */

    struct WindowHint
    {
        int hint;
        int value;
    };

    struct WindowInitArgs
    {
        int width;
        int height;
        bool resizeable;
        const char* title;
    };

    constexpr WindowInitArgs default_args = {.width = 800, .height = 600, .resizeable = true, .title = "Untitled"};

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

        static Window null();

        static void window_resize_callback(GLFWwindow* window, int width, int height);
    private:
        Window(std::nullptr_t);
    };

    /**
     * @}
     */
}

#endif // TOPAZ_CORE_WINDOW_HPP