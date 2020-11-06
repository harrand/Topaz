//
// Created by Harrand on 13/12/2019.
//

#include "glfw_window.hpp"
#include "gl/frame.hpp"
#include "GLFW/glfw3.h"
#include "core/debug/assert.hpp"
#include "core/window.hpp"
#include "dui/imgui_context.hpp"

namespace tz::ext::glfw
{
	namespace detail
	{
		/// Implementation detail. Stores all the tracking data for each active window. Module-wide.
		static std::map<GLFWwindow *, tz::core::GLFWWindow *> window_userdata;
	}
	
	WindowCreationArgs::WindowCreationArgs(): title("Untitled"), width(800), height(600){}

	WindowCreationArgs::WindowCreationArgs(const char* title, int width, int height, bool visible): title(title), width(width), height(height), visible(visible){}

	GLFWWindowImpl::GLFWWindowImpl(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share, bool initially_visible) :
			window_handle(nullptr), title(title), frame(nullptr)
	{
		glfwWindowHint(GLFW_VISIBLE, initially_visible ? GLFW_TRUE : GLFW_FALSE);
		this->window_handle = glfwCreateWindow(width, height, title, monitor, share);
		topaz_assert(this->window_handle != nullptr, "GLFWWindowImpl::GLFWWindowImpl(...): Failed to initialise glfw window!");
		this->frame = std::make_unique<tz::gl::WindowFrame>(this->window_handle);
		glfwSetKeyCallback(this->window_handle, glfw_key_callback);
		glfwSetCharCallback(this->window_handle, glfw_char_callback);
		glfwSetCursorPosCallback(this->window_handle, glfw_mouse_callback);
		glfwSetMouseButtonCallback(this->window_handle, glfw_click_callback);
	}

	GLFWWindowImpl::GLFWWindowImpl(WindowCreationArgs args) : GLFWWindowImpl(args.width, args.height, args.title, nullptr, nullptr, args.visible){}

	GLFWWindowImpl::GLFWWindowImpl(GLFWWindowImpl&& move) noexcept: window_handle(move.window_handle), title(std::move(move.title)), frame(std::move(move.frame))
	{
		move.window_handle = nullptr;
		move.frame = nullptr;
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
		this->title = std::move(move.title);
		this->frame = std::move(move.frame);
		move.window_handle = nullptr;
		move.frame = nullptr;
		return *this;
	}
	
	const std::string& GLFWWindowImpl::get_title() const
	{
		return this->title;
	}
	
	void GLFWWindowImpl::set_title(std::string title)
	{
		this->title = title;
		glfwSetWindowTitle(this->window_handle, this->title.c_str());
	}
	
	int GLFWWindowImpl::get_width() const
	{
		return this->get_size().first;
	}
	
	void GLFWWindowImpl::set_width(int width)
	{
		this->set_size(width, this->get_height());
	}
	
	int GLFWWindowImpl::get_height() const
	{
		return this->get_size().second;
	}
	
	void GLFWWindowImpl::set_height(int height)
	{
		this->set_size(this->get_width(), height);
	}
	
	std::pair<int, int> GLFWWindowImpl::get_size() const
	{
		int w, h;
		glfwGetWindowSize(this->window_handle, &w, &h);
		return {w, h};
	}
	
	void GLFWWindowImpl::set_size(int width, int height)
	{
		glfwSetWindowSize(this->window_handle, width, height);
	}
	
	int GLFWWindowImpl::get_attribute(int attrib_flag) const
	{
		return glfwGetWindowAttrib(this->window_handle, attrib_flag);
	}
	
	void GLFWWindowImpl::set_attribute(int attrib_flag, int value)
	{
		glfwSetWindowAttrib(this->window_handle, attrib_flag, value);
	}
	
	bool GLFWWindowImpl::close_requested() const
	{
		return static_cast<bool>(glfwWindowShouldClose(this->window_handle));
	}
	
	void GLFWWindowImpl::set_close_requested(bool should_close)
	{
		glfwSetWindowShouldClose(this->window_handle, should_close);
	}
	
	void GLFWWindowImpl::swap_buffers() const
	{
		glfwSwapBuffers(this->window_handle);
	}

	tz::gl::IFrame* GLFWWindowImpl::get_frame() const
	{
		return this->frame.get();
	}
	
	void GLFWWindowImpl::register_this(tz::core::GLFWWindow* window)
	{
		detail::window_userdata[this->window_handle] = window;
	}

	bool GLFWWindowImpl::is_registered() const
	{
		return detail::window_userdata.find(this->window_handle) != detail::window_userdata.end();
	}

	bool GLFWWindowImpl::has_active_context() const
	{
		return (this->window_handle == glfwGetCurrentContext());
	}
	
	void GLFWWindowImpl::set_active_context() const
	{
		glfwMakeContextCurrent(this->window_handle);
		tz::ext::imgui::set_window_impl(this->window_handle);
	}
	
	bool GLFWWindowImpl::operator==(const GLFWWindowImpl& rhs) const
	{
		if(this->window_handle == nullptr)
			return false;
		return this->window_handle == rhs.window_handle;
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

	void glfw_mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if(detail::window_userdata.find(window) != detail::window_userdata.end())
		{
			// We know about this window.
			tz::core::GLFWWindow* wnd = detail::window_userdata[window];
			wnd->handle_mouse_event(tz::input::MouseUpdateEvent{xpos, ypos});
		}
	}

	void glfw_click_callback(GLFWwindow* window, int button, int action, int mods)
	{
		if(detail::window_userdata.find(window) != detail::window_userdata.end())
		{
			// We know about this window.
			tz::core::GLFWWindow* wnd = detail::window_userdata[window];
			wnd->handle_click_event(tz::input::MouseClickEvent{button, action, mods});
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