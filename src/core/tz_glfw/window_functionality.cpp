#include "core/tz_glfw/window_functionality.hpp"
#include "algo/container.hpp"
#include "gl/frame.hpp"

namespace tz::ext::glfw
{
    GLFWWindowFunctionality::GLFWWindowFunctionality(GLFWwindow* window): window_handle(nullptr), title(), frame(nullptr)
    {
        this->set_handle(window);
    }

    void GLFWWindowFunctionality::set_handle(GLFWwindow* window)
    {
        this->window_handle = window;
        this->frame = std::make_unique<tz::gl::WindowFrame>(window);
    }

    const char* GLFWWindowFunctionality::get_title() const
    {
        return this->title.c_str();
    }

    void GLFWWindowFunctionality::set_title(const char* title)
    {
        this->title = title;
        glfwSetWindowTitle(this->window_handle, this->title.c_str());
    }

    int GLFWWindowFunctionality::get_width() const
    {
        return this->get_size().first;
    }

    int GLFWWindowFunctionality::get_height() const
    {
        return this->get_size().second;
    }

    void GLFWWindowFunctionality::set_width(int width)
    {
        this->set_size(width, this->get_height());
    }

    void GLFWWindowFunctionality::set_height(int height)
    {
        this->set_size(this->get_width(), height);
    }

    void GLFWWindowFunctionality::set_size(int width, int height)
    {
        glfwSetWindowSize(this->window_handle, width, height);
    }

    tz::gl::IFrame* GLFWWindowFunctionality::get_frame() const
    {
        return this->frame.get();
    }

    bool GLFWWindowFunctionality::is_visible() const
    {
        return static_cast<bool>(this->get_attribute(GLFW_VISIBLE));
    }

    void GLFWWindowFunctionality::set_visible(bool visible)
    {
        this->set_attribute(GLFW_VISIBLE, visible);
    }

    bool GLFWWindowFunctionality::is_resizeable() const
    {
        return static_cast<bool>(this->get_attribute(GLFW_RESIZABLE));
    }

    void GLFWWindowFunctionality::set_resizeable(bool resizeable)
    {
        this->set_attribute(GLFW_RESIZABLE, resizeable);
    }

    bool GLFWWindowFunctionality::is_focused() const
    {
        return static_cast<bool>(this->get_attribute(GLFW_FOCUSED));
    }

    void GLFWWindowFunctionality::set_focused(bool focused)
    {
        this->set_attribute(GLFW_FOCUSED, focused);
    }

    bool GLFWWindowFunctionality::is_close_requested() const
    {
        return static_cast<bool>(glfwWindowShouldClose(this->window_handle));
    }

    void GLFWWindowFunctionality::request_close()
    {
        glfwSetWindowShouldClose(this->window_handle, GLFW_TRUE);
    }

    void GLFWWindowFunctionality::set_active_context()
    {
        glfwMakeContextCurrent(this->window_handle);
    }

    bool GLFWWindowFunctionality::is_active_context() const
    {
        return this->window_handle == glfwGetCurrentContext();
    }

    bool GLFWWindowFunctionality::is_cursor_showing() const
    {
        return glfwGetInputMode(this->window_handle, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
    }

    void GLFWWindowFunctionality::set_cursor_showing(bool show_cursor)
    {
        glfwSetInputMode(this->window_handle, GLFW_CURSOR, show_cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }

    void GLFWWindowFunctionality::update() const
    {
        glfwSwapBuffers(this->window_handle);
    }

    void GLFWWindowFunctionality::handle_key_event(const tz::input::KeyPressEvent& kpe)
    {
        for(const auto& keyl_ptr : this->key_listeners)
        {
            keyl_ptr->on_key_press(kpe);
        }
    }

    void GLFWWindowFunctionality::handle_type_event(const tz::input::CharPressEvent& cpe)
    {
        for(const auto& typel_ptr : this->type_listeners)
        {
            typel_ptr->on_key_type(cpe);
        }
    }

    void GLFWWindowFunctionality::handle_mouse_event(const tz::input::MouseUpdateEvent& mue)
    {
        for(const auto& mousel_ptr : this->mouse_listeners)
        {
            mousel_ptr->on_mouse_update(mue);
        }
    }

    void GLFWWindowFunctionality::handle_click_event(const tz::input::MouseClickEvent& mce)
    {
        for(const auto& mousel_ptr : this->mouse_listeners)
        {
            mousel_ptr->on_mouse_click(mce);
        }
    }

    void GLFWWindowFunctionality::register_key_listener(std::unique_ptr<tz::input::KeyListener> listener)
    {
        if(tz::algo::contains_element(this->key_listeners, listener))
        {
            return;
        }
        this->key_listeners.push_back(std::move(listener));
    }

    void GLFWWindowFunctionality::unregister_key_listener(std::unique_ptr<tz::input::KeyListener> listener)
    {
        this->key_listeners.erase(std::remove(this->key_listeners.begin(), this->key_listeners.end(), listener), this->key_listeners.end());
    }

    void GLFWWindowFunctionality::register_type_listener(std::unique_ptr<tz::input::TypeListener> listener)
    {
        if(tz::algo::contains_element(this->type_listeners, listener))
        {
            return;
        }
        this->type_listeners.push_back(std::move(listener));
    }

    void GLFWWindowFunctionality::unregister_type_listener(std::unique_ptr<tz::input::TypeListener> listener)
    {
        this->type_listeners.erase(std::remove(this->type_listeners.begin(), this->type_listeners.end(), listener), this->type_listeners.end());
    }

    void GLFWWindowFunctionality::register_mouse_listener(std::unique_ptr<tz::input::MouseListener> listener)
    {
        if(tz::algo::contains_element(this->mouse_listeners, listener))
        {
            return;
        }
        this->mouse_listeners.push_back(std::move(listener));
    }

    void GLFWWindowFunctionality::unregister_mouse_listener(std::unique_ptr<tz::input::MouseListener> listener)
    {
        this->mouse_listeners.erase(std::remove(this->mouse_listeners.begin(), this->mouse_listeners.end(), listener), this->mouse_listeners.end());
    }

    std::pair<int, int> GLFWWindowFunctionality::get_size() const
    {
        int w, h;
		glfwGetWindowSize(this->window_handle, &w, &h);
		return {w, h};
    }

    int GLFWWindowFunctionality::get_attribute(int attrib_type) const
    {
        return glfwGetWindowAttrib(this->window_handle, attrib_type);
    }

    void GLFWWindowFunctionality::set_attribute(int attrib_type, int value)
    {
        glfwSetWindowAttrib(this->window_handle, attrib_type, value);
    }
}
