#ifndef TOPAZ_GL_IMGUI_CONTEXT_HPP
#define TOPAZ_GL_IMGUI_CONTEXT_HPP

// Forward Declares
struct GLFWwindow;

class ImGuiWindow
{
public:
    ImGuiWindow();
    virtual void render() = 0;
};

namespace tz::ext::imgui
{
    static GLFWwindow* window;
    void set_window_impl(GLFWwindow* wnd);
    void initialise();
    void terminate();
    template<class WindowType, typename... Args>
    WindowType& emplace_window(Args&&... args);
    void update();
}

#endif // TOPAZ_GL_IMGUI_CONTEXT_HPP
#include "gl/tz_imgui/imgui_context.inl"