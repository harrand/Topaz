//
// Created by Harrand on 13/12/2019.
//

#include "core/core.hpp"
#include "glad_context.hpp"
#include "core/debug/assert.hpp"

#include "GLFW/glfw3.h"

namespace tz::ext::glad
{
	GLADContext::GLADContext(const tz::ext::glfw::GLFWContext& glfw_context) noexcept: glfw_context(&glfw_context), loaded(false){}
	
	GLADContext::GLADContext() noexcept: glfw_context(nullptr), loaded(false){}
	
	void GLADContext::load()
	{
		if(this->glfw_context == nullptr)
		{
			// If we never were given a glfw context, use the global one now.
			this->glfw_context = &tz::ext::glfw::get();
		}
		topaz_assert(!this->loaded, "GLADContext::load(): Context already marked as loaded!");
		topaz_assert(this->glfw_context->has_window(), "GLADContext::load(): Given GLFW context is incomplete -- It doesn't have a valid window attached to it.");
		// Load all the things!
		gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
	}
	
	bool GLADContext::is_loaded() const
	{
		return this->loaded;
	}
	
	void load_opengl()
	{
		topaz_assert(!global_context.is_loaded(), "tz::ext::glad::load_opengl(): Global GLAD context is already loaded!");
		global_context = GLADContext{}; // Re-assign incase a new glfw context was since bound.
		global_context.load(); // Let's go!
	}
	
	void load_opengl(const tz::ext::glfw::GLFWContext& specific_context)
	{
		topaz_assert(!global_context.is_loaded(), "tz::ext::glad::load_opengl(): Global GLAD context is already loaded!");
		global_context = GLADContext{specific_context}; // Re-assign using the context we were told to use.
		global_context.load(); // Let's go!
	}
}