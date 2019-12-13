//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_GLFW_WINDOW_HPP
#define TOPAZ_GLFW_WINDOW_HPP
// Forward declares
struct GLFWmonitor;
struct GLFWwindow;

namespace tz::ext::glfw
{
    struct WindowCreationArgs
    {
        WindowCreationArgs();
        WindowCreationArgs(const char* title, int width, int height);

        const char* title;
        int width;
        int height;
    };
    class GLFWWindowImpl
    {
    public:
        // No copying, only moving allowed.
        GLFWWindowImpl(const GLFWWindowImpl& copy) = delete;
        GLFWWindowImpl(GLFWWindowImpl&& move) noexcept;
        ~GLFWWindowImpl();
        GLFWWindowImpl& operator=(const GLFWWindowImpl& copy) = delete;
        GLFWWindowImpl& operator=(GLFWWindowImpl&& move);

        friend void initialise(WindowCreationArgs);
    private:
        GLFWWindowImpl(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
        explicit GLFWWindowImpl(WindowCreationArgs args);

        GLFWwindow* window_handle;
    };
}


#endif //TOPAZ_GLFW_WINDOW_HPP
