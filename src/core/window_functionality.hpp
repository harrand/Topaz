#ifndef TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP
#define TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP
#include "GLFW/glfw3.h"

namespace tz
{
    class WindowFunctionality
    {
    public:
        WindowFunctionality(GLFWwindow* wnd);
        GLFWwindow* get_middleware_handle() const;
        bool is_close_requested() const;

        void update();
    protected:
        GLFWwindow* wnd;
    private:
        void ensure() const;
    };
}

#endif // TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP