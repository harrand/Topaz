#include "core/window.hpp"
#include "GLFW/glfw3.h"
#include "core/profiling/zone.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include <utility>

namespace tz
{
	Window::Window(WindowInitArgs args, WindowHintList hints):
	WindowFunctionality(nullptr)
	{
		TZ_PROFZONE("tz::Window Create", TZ_PROFCOL_BLUE);
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
		if(TZ_DEBUG && this->wnd == nullptr)
		{
			// We failed somehow.	
			switch(glfwGetError(nullptr))
			{
				case GLFW_NOT_INITIALIZED:
					tz_error("Failed to create window because glfw was not initialised correctly.");
				break;
				case GLFW_INVALID_ENUM:
					tz_error("Failed to create window due to invalid glfw enum.");
				break;
				case GLFW_INVALID_VALUE:
					tz_error("Failed to create window due to invalid glfw value.");
				break;
				case GLFW_API_UNAVAILABLE:
					tz_error("Failed to create window because GLFW could not find support for the requested API on the system.");
				break;
				case GLFW_VERSION_UNAVAILABLE:
					tz_error("Failed to create window because the requested OpenGL version (4.6) is not available on this machine. If this is a vulkan build, please submit a bug report.");
				break;
				case GLFW_FORMAT_UNAVAILABLE:
					tz_error("Failed to create window because the requested pixel format is not supported.");
				break;
				case GLFW_PLATFORM_ERROR:
					tz_error("Failed to create window because a miscellaneous platform-specific error occurred. Either a bug/error in the config of GLFW has taken place, or the machine's OS/drivers are misconfigured, or there was a lack of required resources. Please submit a bug report.");
				break;
				default:
					tz_error("Failed to create window, but for unknown reason. GLFW has returned an undocumented error code for glfwCreateWindow, so something is very wrong. Please submit a bug report. Error code %d", glfwGetError(nullptr));
				break;
			}
		}
		glfwMakeContextCurrent(this->wnd);
		#if TZ_OGL
			// Vulkan isn't fps-capped -- neither should opengl be.
			glfwSwapInterval(0);
		#endif
		glfwSetWindowUserPointer(this->wnd, this);
		glfwSetFramebufferSizeCallback(this->wnd, Window::window_resize_callback);
		glfwSetWindowPosCallback(this->wnd, Window::window_move_callback);
		glfwSetKeyCallback(this->wnd, Window::key_callback);
		glfwSetMouseButtonCallback(this->wnd, Window::mouse_button_callback);
	}

	Window::Window(Window&& move):
	WindowFunctionality(nullptr)
	{
		std::swap(this->wnd, move.wnd);
	}

	Window::~Window()
	{
		TZ_PROFZONE("tz::Window Destroy", TZ_PROFCOL_BLUE);
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
		cur_window->on_resize()(tz::Vec2ui{static_cast<unsigned int>(width), static_cast<unsigned int>(height)});
	}

	void Window::window_move_callback(GLFWwindow* window, int width, int height)
	{
		WindowFunctionality* cur_window_func = reinterpret_cast<WindowFunctionality*>(glfwGetWindowUserPointer(window));
		Window* cur_window = static_cast<Window*>(cur_window_func);
		cur_window->on_move()(tz::Vec2ui{static_cast<unsigned int>(width), static_cast<unsigned int>(height)});
	}

	void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		WindowFunctionality* cur_window_func = reinterpret_cast<WindowFunctionality*>(glfwGetWindowUserPointer(window));
		static_cast<Window*>(cur_window_func)->handle_key_event(key, scancode, action, mods);
	}

	void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		WindowFunctionality* cur_window_func = reinterpret_cast<WindowFunctionality*>(glfwGetWindowUserPointer(window));
		static_cast<Window*>(cur_window_func)->handle_mouse_event(button, action, mods);
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
