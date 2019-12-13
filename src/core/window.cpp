//
// Created by Harrand on 13/12/2019.
//

#include "window.hpp"
#include "core/debug/assert.hpp"
#include "GLFW/glfw3.h"

namespace tz::core
{
	GLFWWindow::GLFWWindow(tz::ext::glfw::GLFWContext& ctx): impl(ctx.get_window())
	{
		this->set_active_context();
	}

	const char* GLFWWindow::get_title() const
	{
		this->verify();
		return this->impl->title;
	}
	
	void GLFWWindow::set_title(const char* title)
	{
		this->verify();
		glfwSetWindowTitle(this->impl->window_handle, title);
	}
	
	int GLFWWindow::get_width() const
	{
		return this->get_size().first;
	}
	
	int GLFWWindow::get_height() const
	{
		return this->get_size().second;
	}
	
	void GLFWWindow::set_width(int width) const
	{
		int h = this->get_height();
		glfwSetWindowSize(this->impl->window_handle, width, h);
	
	}
	void GLFWWindow::set_height(int height) const
	{
		int w = this->get_width();
		glfwSetWindowSize(this->impl->window_handle, w, height);
	}
	
	void GLFWWindow::set_size(int width, int height) const
	{
		this->verify();
		glfwSetWindowSize(this->impl->window_handle, width, height);
	}
	
	bool GLFWWindow::is_visible() const
	{
		this->verify();
		return static_cast<bool>(glfwGetWindowAttrib(this->impl->window_handle, GLFW_VISIBLE));
	}
	
	void GLFWWindow::set_visible(bool visible) const
	{
		this->verify();
		glfwSetWindowAttrib(this->impl->window_handle, GLFW_VISIBLE, visible);
	}
	
	bool GLFWWindow::is_resizeable() const
	{
		this->verify();
		return static_cast<bool>(glfwGetWindowAttrib(this->impl->window_handle, GLFW_RESIZABLE));
	}
	
	void GLFWWindow::set_resizeable(bool resizeable) const
	{
		this->verify();
		glfwSetWindowAttrib(this->impl->window_handle, GLFW_RESIZABLE, resizeable);
	}
	
	bool GLFWWindow::is_focused() const
	{
		this->verify();
		return static_cast<bool>(glfwGetWindowAttrib(this->impl->window_handle, GLFW_FOCUSED));
	}
	
	void GLFWWindow::set_focused(bool focused) const
	{
		this->verify();
		glfwSetWindowAttrib(this->impl->window_handle, GLFW_FOCUSED, focused);
	}
	
	bool GLFWWindow::is_close_requested() const
	{
		this->verify();
		return static_cast<bool>(glfwWindowShouldClose(this->impl->window_handle));
	}
	
	void GLFWWindow::request_close() const
	{
		this->verify();
		glfwSetWindowShouldClose(this->impl->window_handle, true);
	
	}
	
	std::pair<int, int> GLFWWindow::get_size() const
	{
		this->verify();
		int w, h;
		glfwGetWindowPos(this->impl->window_handle, &w, &h);
		return {w, h};
	}
	
	void GLFWWindow::verify() const
	{
		#if TOPAZ_DEBUG
			topaz_assert(this->impl != nullptr, "GLFWWindow::verify(): Verification failed. Did you tz::initialise()?");
		#endif
	}
	
	void GLFWWindow::set_active_context() const
	{
		this->verify();
		glfwMakeContextCurrent(this->impl->window_handle);
	}
	
	bool GLFWWindow::is_active_context() const
	{
		this->verify();
		return this->impl->has_active_context();
	}
	
	void GLFWWindow::update() const
	{
		this->verify();
		glfwSwapBuffers(this->impl->window_handle);
		glfwPollEvents();
	}
}