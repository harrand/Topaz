#ifndef TOPAZ_GL_IMGUI_CONTEXT_HPP
#define TOPAZ_GL_IMGUI_CONTEXT_HPP
#include "imgui.h"
#include <memory>


// Forward Declares
struct GLFWwindow;

namespace tz::ext::imgui
{
    class ImGuiWindow
    {
    public:
        virtual void render() = 0;
    };

    static GLFWwindow* window;
    void set_window_impl(GLFWwindow* wnd);
    void initialise();
    void terminate();
    template<class WindowType, typename... Args>
    WindowType& emplace_window(Args&&... args);
    ImGuiWindow& add_window(std::unique_ptr<ImGuiWindow> wnd);
    void update();
}

#endif // TOPAZ_GL_IMGUI_CONTEXT_HPP
#include "gl/tz_imgui/imgui_context.inl"