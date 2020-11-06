#include "gl/screen.hpp"
#include "core/tz_glfw/glfw_context.hpp"

namespace tz::gl
{
    /*static*/ Screen Screen::primary()
    {
        return {tz::ext::glfw::get_monitor_width(), tz::ext::glfw::get_monitor_height()};
    }

    int Screen::get_width() const
    {
        return this->width;
    }
    
    int Screen::get_height() const
    {
        return this->height;
    }
    
    float Screen::get_aspect_ratio() const
    {
        return static_cast<float>(this->width) / this->height;
    }

    Screen::Screen(int width, int height): width(width), height(height){}
}