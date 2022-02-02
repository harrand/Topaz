#include "core/window.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include <utility>

namespace tz
{
	Window::Window(WindowInitArgs args, WindowHintList hints):
	WindowFunctionality(nullptr)
	{
		if(!args.flags.resizeable)
		{
			// GLFW assumes window is resizable. If not we will need to add an extra hint.
			hints.add(WindowHint{.hint = GLFW_RESIZABLE, .value = GLFW_FALSE});
		}
		for(const WindowHint& hint : hints)
		{
			glfwWindowHint(hint.hint, hint.value);
		}

		#if TZ_VULKAN
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		#elif TZ_OGL
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, tz::gl::ogl2::ogl_version.major);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, tz::gl::ogl2::ogl_version.minor);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			#if TZ_DEBUG
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
			#else
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
			#endif
		#endif

		this->wnd = glfwCreateWindow(args.width, args.height, args.title, nullptr, nullptr);
		glfwMakeContextCurrent(this->wnd);
		#if TZ_OGL
			// Vulkan isn't fps-capped -- neither should opengl be.
			glfwSwapInterval(0);
		#endif
		glfwSetWindowUserPointer(this->wnd, this);
		glfwSetFramebufferSizeCallback(this->wnd, Window::window_resize_callback);
		glfwSetKeyCallback(this->wnd, Window::key_callback);
	}

	Window::Window(Window&& move):
	WindowFunctionality(nullptr)
	{
		std::swap(this->wnd, move.wnd);
	}

	Window::~Window()
	{
		if(this->wnd != nullptr)
		{
			glfwDestroyWindow(this->wnd);
			this->wnd = nullptr;
		}
	}

	Window& Window::operator=(Window&& rhs)
	{
		std::swap(this->wnd, rhs.wnd);
		return *this;
	}

	Window Window::null()
	{
		return {nullptr};
	}

	bool Window::is_null() const
	{
		return this->wnd == nullptr;	
	}

	void Window::window_resize_callback(GLFWwindow* window, int width, int height)
	{
		WindowFunctionality* cur_window_func = reinterpret_cast<WindowFunctionality*>(glfwGetWindowUserPointer(window));
		Window* cur_window = static_cast<Window*>(cur_window_func);
		for(const auto& resize_callback : cur_window->window_resize_callbacks)
		{
			resize_callback(width, height);
		}
	}

	void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		WindowFunctionality* cur_window_func = reinterpret_cast<WindowFunctionality*>(glfwGetWindowUserPointer(window));
		static_cast<Window*>(cur_window_func)->handle_key_event(key, scancode, action, mods);
	}

	void Window::handle_key_event(int key, int scancode, int action, int mods)
	{
		WindowFunctionality::handle_key_event(key, scancode, action, mods);
	}


	Window::Window(std::nullptr_t):
	WindowFunctionality(nullptr)
	{
		// Empty window, breaks if you try and do anything with it.
	}

}
