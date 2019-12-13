//
// Created by Harrand on 13/12/2019.
//

#include "core/debug/assert.hpp"
#include "core/debug/print.hpp"
#include "glfw_context.hpp"
#include "GLFW/glfw3.h"

namespace tz::ext::glfw
{
	GLFWContext::GLFWContext() noexcept: initialised(false), args(), window({std::nullopt}){}

	void GLFWContext::init()
	{
		topaz_assert(!this->initialised, "GLFWContext::initialise(): Initialised flag is already set!");
		[[maybe_unused]] int success = glfwInit();
		topaz_assert(success, "GLFWContext::initialise(): glfwInit() failed!");
		this->initialised = true;

		tz::debug_printf("glfwInit() returned %d\n", success);
	}

	void GLFWContext::term()
	{
		topaz_assert(this->initialised, "GLFWContext::terminate(): Initialised flag wasn't set!");
		glfwTerminate();
		this->initialised = false;

		tz::debug_printf("glfwTerminate() invoked\n");
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

	void GLFWContext::set_window(tz::ext::glfw::GLFWWindowImpl&& window_to_claim)
	{
		this->window = std::move(window_to_claim);
	}

	static GLFWContext glfw;
	void initialise(WindowCreationArgs args)
	{
		glfw.init();
		glfw.set_window(GLFWWindowImpl{args});

	}

	void terminate()
	{
		glfw.term();
	}

	GLFWContext& get()
	{
		return glfw;
	}
}