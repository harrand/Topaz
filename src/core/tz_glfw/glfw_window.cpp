//
// Created by Harrand on 13/12/2019.
//

#include "glfw_window.hpp"
#include "GLFW/glfw3.h"
#include "core/debug/assert.hpp"
#include "core/window.hpp"

namespace tz::ext::glfw
{
	WindowCreationArgs::WindowCreationArgs(): title("Untitled"), width(800), height(600){}

	WindowCreationArgs::WindowCreationArgs(const char* title, int width, int height): title(title), width(width), height(height){}

	GLFWWindowImpl::GLFWWindowImpl(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) :
			window_handle(glfwCreateWindow(width, height, title, monitor, share)), title(title)
	{
		topaz_assert(this->window_handle != nullptr, "GLFWWindowImpl::GLFWWindowImpl(...): Failed to initialise glfw window!");
		glfwSetKeyCallback(this->window_handle, glfw_key_callback);
		glfwSetCharCallback(this->window_handle, glfw_char_callback);
	}

	GLFWWindowImpl::GLFWWindowImpl(WindowCreationArgs args) : GLFWWindowImpl(args.width, args.height, args.title, nullptr, nullptr){}

	GLFWWindowImpl::GLFWWindowImpl(GLFWWindowImpl&& move) noexcept: window_handle(move.window_handle)
	{
		move.window_handle = nullptr;
	}

	GLFWWindowImpl::~GLFWWindowImpl()
	{
		if(this->window_handle != nullptr)
		{
			glfwDestroyWindow(this->window_handle);
		}
	}

	GLFWWindowImpl& GLFWWindowImpl::operator=(GLFWWindowImpl&& move) noexcept
	{
		topaz_assert(move.window_handle != nullptr, "GLFWWindowImpl::operator=(move): Invoked with nullified window param. This is wrong.");
		this->window_handle = move.window_handle;
		move.window_handle = nullptr;
	}
	
	void GLFWWindowImpl::register_this(tz::core::GLFWWindow* window)
	{
		detail::window_userdata[this->window_handle] = window;
	}

	bool GLFWWindowImpl::has_active_context() const
	{
		return (this->window_handle == glfwGetCurrentContext());
	}
	
	void register_window(tz::core::GLFWWindow* window)
	{
		topaz_assert(window != nullptr, "tz::ext::glfw::register_window(GLFWWindow): Attempting to register a null window!");
		window->impl->register_this(window);
	}
	
	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if(detail::window_userdata.find(window) != detail::window_userdata.end())
		{
			// We know about this window.
			tz::core::GLFWWindow* wnd = detail::window_userdata[window];
			wnd->handle_key_event(tz::input::KeyPressEvent{key, scancode, action, mods});
		}
	}
	
	void glfw_char_callback(GLFWwindow* window, unsigned int codepoint)
	{
		if(detail::window_userdata.find(window) != detail::window_userdata.end())
		{
			// We know about this window.
			tz::core::GLFWWindow* wnd = detail::window_userdata[window];
			wnd->handle_type_event(tz::input::CharPressEvent{codepoint});
		}
	}
	
	void simulate_key_press(const tz::input::KeyPressEvent& kpe)
	{
		glfw_key_callback(glfwGetCurrentContext(), kpe.key, kpe.scancode, kpe.action, kpe.mods);
	}
	
	void simulate_key_type(const tz::input::CharPressEvent& cpe)
	{
		glfw_char_callback(glfwGetCurrentContext(), cpe.codepoint);
	}
	
	void simulate_typing(const char* letters)
	{
		while(*letters != '\0')
		{
			tz::input::CharPressEvent cpe = {static_cast<unsigned int>(*letters)};
			simulate_key_type(cpe);
			letters++;
		}
	}
}