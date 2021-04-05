#ifndef TOPAZ_CORE_TZ_GLFW_WINDOW_IMPL_HPP
#define TOPAZ_CORE_TZ_GLFW_WINDOW_IMPL_HPP
#include "ext/tz_glfw/window_functionality.hpp"
#include <vector>
#include <initializer_list>

namespace tz::ext::glfw
{
    struct WindowHint
    {
        int hint;
        int value;
    };

    class WindowHintCollection
    {
    public:
        WindowHintCollection() = default;
        WindowHintCollection(std::initializer_list<WindowHint> hints);
        void add(int hint, int value);
        auto begin();
        auto end();
    private:
        std::vector<WindowHint> hints;
    };

    struct WindowCreationArgs
    {
        const char* title;
        int width;
        int height;
    };

    class WindowImpl
    {
    public:
        WindowImpl(WindowCreationArgs args);
        WindowImpl(WindowCreationArgs args, WindowHintCollection hints);
        ~WindowImpl();

        GLFWWindowFunctionality& functionality();

        friend void set_active(const WindowImpl& impl);
        friend const WindowImpl& get_active();

        void glfw_key_callback(int key, int scancode, int action, int mods);  
        void glfw_char_callback(unsigned int codepoint);   
        void glfw_cursor_pos_callback(double xpos, double ypos);
        void glfw_mouse_button_callback(int button, int action, int mods); 
    private:
        void setup_callbacks();
        GLFWwindow* window_handle;
        GLFWWindowFunctionality wnd_functionality;
    };

    void common_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void common_char_callback(GLFWwindow* window, unsigned int codepoint);
    void common_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    void common_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
}

#endif // TOPAZ_CORE_TZ_GLFW_WINDOW_IMPL_HPP