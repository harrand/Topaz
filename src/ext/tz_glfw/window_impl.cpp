#include "ext/tz_glfw/window_impl.hpp"
#include "core/debug/assert.hpp"
#include "dui/window.hpp"

namespace tz::ext::glfw
{
    WindowHintCollection::WindowHintCollection(std::initializer_list<WindowHint> hints): hints(hints){}
    void WindowHintCollection::add(int hint, int value)
    {
        this->hints.emplace_back(hint, value);
    }

    auto WindowHintCollection::begin()
    {
        return this->hints.begin();
    }

    auto WindowHintCollection::end()
    {
        return this->hints.end();
    }

    WindowImpl::WindowImpl(WindowCreationArgs args): WindowImpl{args, WindowHintCollection{}}{}

    WindowImpl::WindowImpl(WindowCreationArgs args, WindowHintCollection hints): window_handle(nullptr), wnd_functionality()
    {
        for(const WindowHint& hint : hints)
        {
            glfwWindowHint(hint.hint, hint.value);
        }

        // This creates the window aswell as the context.
        this->window_handle = glfwCreateWindow(args.width, args.height, args.title, nullptr, nullptr);
        topaz_hard_assert(this->window_handle != nullptr, "tz::ext::glfw::WindowImpl(args, hints): Window or OpenGL context creation failed.");
        tz::dui::set_window_impl(this->window_handle);
        // Setup GLFW callbacks.
        this->setup_callbacks();

        // Setup window functionality.
        this->wnd_functionality.set_handle(this->window_handle);
        this->wnd_functionality.set_title(args.title);
    }

    WindowImpl::~WindowImpl()
    {
        topaz_assert(this->window_handle != nullptr, "tz::ext::glfw::WindowImpl::~WindowImpl(): Window and OpenGL context were null prior to destruction.");
        glfwDestroyWindow(this->window_handle);
    }

    GLFWWindowFunctionality& WindowImpl::functionality()
    {
        return this->wnd_functionality;
    }

    void WindowImpl::glfw_key_callback(int key, int scancode, int action, int mods)
    {
        auto int_to_key_action = [](int a)->tz::input::KeyPressEvent::Action
			{
				switch(a)
				{
					case GLFW_PRESS:
						return tz::input::KeyPressEvent::Action::Pressed;
					break;
					case GLFW_RELEASE:
						return tz::input::KeyPressEvent::Action::Released;
					break;
					case GLFW_REPEAT:
						return tz::input::KeyPressEvent::Action::Repeat;
					break;
				}
				topaz_assert(false, "");
				return tz::input::KeyPressEvent::Action::Released;
			};
        this->wnd_functionality.handle_key_event(tz::input::KeyPressEvent{key, scancode, int_to_key_action(action), mods});
    }

    void WindowImpl::glfw_char_callback(unsigned int codepoint)
    {
        this->wnd_functionality.handle_type_event(tz::input::CharPressEvent{codepoint});
    }

    void WindowImpl::glfw_cursor_pos_callback(double xpos, double ypos)
    {
        this->wnd_functionality.handle_mouse_event(tz::input::MouseUpdateEvent{xpos, ypos});
    }

    void WindowImpl::glfw_mouse_button_callback(int button, int action, int mods)
    {
        this->wnd_functionality.handle_click_event(tz::input::MouseClickEvent{button, action, mods});
    }

    void WindowImpl::setup_callbacks()
    {
        glfwSetWindowUserPointer(this->window_handle, this);
        glfwSetKeyCallback(this->window_handle, common_key_callback);
        glfwSetCharCallback(this->window_handle, common_char_callback);
        glfwSetCursorPosCallback(this->window_handle, common_cursor_pos_callback);
        glfwSetMouseButtonCallback(this->window_handle, common_mouse_button_callback);
    }

    WindowImpl* get_wnd(GLFWwindow* window_handle)
    {
        return static_cast<WindowImpl*>(glfwGetWindowUserPointer(window_handle));
    }

    void common_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        get_wnd(window)->glfw_key_callback(key, scancode, action, mods);
    }

    void common_char_callback(GLFWwindow* window, unsigned int codepoint)
    {
        get_wnd(window)->glfw_char_callback(codepoint);
    }

    void common_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
    {
        get_wnd(window)->glfw_cursor_pos_callback(xpos, ypos);
    }

    void common_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        get_wnd(window)->glfw_mouse_button_callback(button, action, mods);
    }
}