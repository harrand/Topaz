//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_GLFW_CONTEXT_HPP
#define TOPAZ_GLFW_CONTEXT_HPP
#include "core/tz_glfw/glfw_window.hpp"
#include <optional>

namespace tz::ext::glfw
{
    class GLFWContext
    {
    public:
        GLFWContext() noexcept;
        void init();
        void term();
        tz::ext::glfw::GLFWWindowImpl* get_window();
        const tz::ext::glfw::GLFWWindowImpl* get_window() const;
        bool has_window() const;

        friend void initialise(WindowCreationArgs);
    private:
        void set_window(tz::ext::glfw::GLFWWindowImpl&& window);

        bool initialised;
        tz::ext::glfw::WindowCreationArgs args;
        std::optional<tz::ext::glfw::GLFWWindowImpl> window;
    };

    void initialise(WindowCreationArgs args);
    void terminate();
    const GLFWContext& get();
}

#endif //TOPAZ_GLFW_CONTEXT_HPP
