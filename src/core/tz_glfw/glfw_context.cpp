//
// Created by Harrand on 13/12/2019.
//

#include "core/debug/assert.hpp"
#include "core/debug/print.hpp"
#include "glfw_context.hpp"
#include "GLFW/glfw3.h"
#include <vector>
#include <algorithm>

namespace tz::ext::glfw
{
	namespace detail
	{
		//void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, void *userParam);
	}

	GLFWContext::GLFWContext() noexcept: initialised(false), args(), window({std::nullopt}), error_callback(nullptr){}

	void GLFWContext::init()
	{
		topaz_assert(!this->initialised, "GLFWContext::initialise(): Initialised flag is already set!");
		[[maybe_unused]] int success = glfwInit();
		topaz_assert(success, "GLFWContext::initialise(): glfwInit() failed!");
		this->initialised = true;

		glfwSwapInterval(0);
	}

	void GLFWContext::term()
	{
		topaz_assert(this->initialised, "GLFWContext::terminate(): Initialised flag wasn't set!");
		glfwTerminate();
		this->initialised = false;
	}

	tz::ext::glfw::GLFWWindowImpl* GLFWContext::get_window()
	{
		if(this->window.has_value())
		{
			return &this->window.value();
		}

		return nullptr;
	}

	const tz::ext::glfw::GLFWWindowImpl* GLFWContext::get_window() const
	{
		if(this->window.has_value())
		{
			return &this->window.value();
		}

		return nullptr;
	}

	bool GLFWContext::has_window() const
	{
		return this->window.has_value();
	}

	bool GLFWContext::is_active_context() const
	{
		if(this->has_window())
		{
			return this->get_window()->has_active_context();
		}
		return false;
	}

	void GLFWContext::set_active_context() const
	{
		if(this->has_window())
		{
			this->get_window()->set_active_context();
			glfwSetErrorCallback(this->error_callback);
		}
	}
	
	GLFWErrorCallback* GLFWContext::get_error_callback() const
	{
		return this->error_callback;
	}
		
	void GLFWContext::set_error_callback(GLFWErrorCallback* callback)
	{
		this->error_callback = callback;
	}

	bool GLFWContext::operator==(const GLFWContext& rhs) const
	{
		return this->window == rhs.window;
	}
	
	GLFWContext::GLFWContext(bool pre_initialised) noexcept: GLFWContext()
	{
		this->initialised = pre_initialised;
	}

	void GLFWContext::set_window(tz::ext::glfw::GLFWWindowImpl&& window_to_claim)
	{
		this->window = std::move(window_to_claim);
	}

	static GLFWContext glfw;
	static std::vector<GLFWContext> extra_contexts;
	
	GLFWWindowImpl make_impl(WindowCreationArgs args)
	{
		return GLFWWindowImpl{args};
	}
	
	void give_window(GLFWContext& ctx, GLFWWindowImpl&& impl)
	{
		ctx.set_window(std::move(impl));
	}

	void check_one_monitor()
	{
		#if TOPAZ_DEBUG
			int count;
			glfwGetMonitors(&count);
			if(count > 1)
			{
				tz::debug_printf("Detected %d monitors. Only one monitor is supported; tz::gl may not perform correctly.");
			}
		#endif
	}

	std::pair<int, int> get_monitor_dimensions()
	{
		check_one_monitor();
		GLFWmonitor* default_monitor = glfwGetPrimaryMonitor();
		topaz_assert(default_monitor != nullptr, "glfwGetPrimaryMonitor() returned nullptr");
		const GLFWvidmode* vm = glfwGetVideoMode(default_monitor);
		return {vm->width, vm->height};
	}

	int get_monitor_width()
	{
		return get_monitor_dimensions().first;
	}

	int get_monitor_height()
	{
		return get_monitor_dimensions().second;
	}
	
	void initialise(WindowCreationArgs args)
	{
		glfw.init();
		give_window(glfw, make_impl(args));
	}

	void terminate()
	{
		glfw.term();
	}

	GLFWContext& get()
	{
		return glfw;
	}
	
	GLFWContext& make_secondary_context(WindowCreationArgs args)
	{
		GLFWContext secondary_context{true};
		give_window(secondary_context, make_impl(args));
		extra_contexts.push_back(std::move(secondary_context));
		return extra_contexts.back();
	}
	
	void destroy_secondary_context(GLFWContext& context)
	{
		extra_contexts.erase(std::remove(extra_contexts.begin(), extra_contexts.end(), context), extra_contexts.end());
	}
}