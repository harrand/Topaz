//
// Created by Harrand on 13/12/2019.
//

#include "glfw_window.hpp"
#include "GLFW/glfw3.h"
#include "core/debug/assert.hpp"

namespace tz::ext::glfw
{
	WindowCreationArgs::WindowCreationArgs(): title("Untitled"), width(800), height(600){}

	WindowCreationArgs::WindowCreationArgs(const char* title, int width, int height): title(title), width(width), height(height){}

	GLFWWindowImpl::GLFWWindowImpl(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) :
			window_handle(glfwCreateWindow(width, height, title, monitor, share)), title(title)
	{
		topaz_assert(this->window_handle != nullptr, "GLFWWindowImpl::GLFWWindowImpl(...): Failed to initialise glfw window!");
	}

	GLFWWindowImpl::GLFWWindowImpl(WindowCreationArgs args) : GLFWWindowImpl(args.width, args.height, args.title, nullptr, nullptr){}

	GLFWWindowImpl::GLFWWindowImpl(GLFWWindowImpl&& move) noexcept: window_handle(move.window_handle)
	{
		move.window_handle = nullptr;
	}

	GLFWWindowImpl::~GLFWWindowImpl()
	{
		glfwDestroyWindow(this->window_handle);
	}

	GLFWWindowImpl& GLFWWindowImpl::operator=(GLFWWindowImpl&& move)
	{
		topaz_assert(move.window_handle != nullptr, "GLFWWindowImpl::operator=(move): Invoked with nullified window param. This is wrong.");
		this->window_handle = move.window_handle;
		move.window_handle = nullptr;
	}

	bool GLFWWindowImpl::has_active_context() const
	{
		return (this->window_handle == glfwGetCurrentContext());
	}
	
}