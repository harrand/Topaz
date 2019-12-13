//
// Created by Harrand on 13/12/2019.
//

#include "window.hpp"

namespace tz::core
{
    GLFWWindow::GLFWWindow(tz::ext::glfw::GLFWWindowImpl&& impl): impl(std::move(impl))
    {

    }
}