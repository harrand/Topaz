//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_WINDOW_HPP
#define TOPAZ_WINDOW_HPP
#include "core/tz_glfw/glfw_context.hpp"

namespace tz::core
{
    class IWindow
    {
    public:
        virtual const char* get_name() const = 0;
        virtual void set_name(const char* name) = 0;
        virtual int get_width() const = 0;
        virtual int get_height() const = 0;
        virtual void set_width() const = 0;
        virtual void set_height() const = 0;
        virtual bool is_visible() const = 0;
        virtual void set_visible() const = 0;
        virtual bool is_close_requested() const = 0;
        virtual void request_close() const = 0;
    };


    class GLFWWindow : public IWindow
    {
    public:
        explicit GLFWWindow(tz::ext::glfw::GLFWWindowImpl&& impl);
    private:
        tz::ext::glfw::GLFWWindowImpl impl;
    };
}


#endif //TOPAZ_WINDOW_HPP
