#include "core/window_functionality.hpp"
#include "core/assert.hpp"
#include "core/profiling/zone.hpp"

#if TZ_OGL
#include "glad/glad.h"
#endif

namespace tz
{
	WindowFunctionality::WindowFunctionality(GLFWwindow* wnd):
	wnd(wnd){}

	GLFWwindow* WindowFunctionality::get_middleware_handle() const
	{
		return this->wnd;
	}

	bool WindowFunctionality::is_close_requested() const
	{
		this->ensure();
		return glfwWindowShouldClose(this->wnd) == GLFW_TRUE;
	}

	float WindowFunctionality::get_width() const
	{
		return static_cast<float>(this->get_size().first);
	}

	float WindowFunctionality::get_height() const
	{
		return static_cast<float>(this->get_size().second);
	}

	bool WindowFunctionality::is_key_pressed(int key_code) const
	{
		return this->pressed.is_pressed(key_code);
	}

	void WindowFunctionality::update()
	{
		TZ_PROFZONE("WindowFunctionality::update", TZ_PROFCOL_YELLOW);
		glfwPollEvents();
		#if TZ_OGL
			// OpenGL only
			glfwSwapBuffers(this->wnd);
		#endif
	}

	/*
	void WindowFunctionality::set_render_target() const
	{
		#if TZ_VULKAN

		#elif TZ_OGL
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, static_cast<GLsizei>(this->get_width()), static_cast<GLsizei>(this->get_height()));
		#endif
	}
	*/

	void WindowFunctionality::block_until_event_happens()
	{
		glfwWaitEvents();
	}

	void WindowFunctionality::handle_key_event(int key, int scancode, int action, int mods)
	{
		this->pressed.glfw_update_state(key, scancode, action, mods);
	}

	std::pair<int, int> WindowFunctionality::get_size() const
	{
		int w, h;
		glfwGetFramebufferSize(this->wnd, &w, &h);
		return {w, h};
	}

	void WindowFunctionality::ensure() const
	{
		tz_assert(this->wnd != nullptr, "WindowFunctionality::ensure(): Failed");
	}
}
