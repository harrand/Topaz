#include "gl/frame.hpp"
#include "core/debug/assert.hpp"
#include "glfw/glfw3.h"

namespace tz::gl
{
	IFrame::IFrame(unsigned int width, unsigned int height): width(width), height(height){}

	unsigned int IFrame::get_width() const
	{
		return this->width;
	}

	unsigned int IFrame::get_height() const
	{
		return this->height;
	}

	void IFrame::set_width(unsigned int width)
	{
		this->width = width;
	}

	void IFrame::set_height(unsigned int height)
	{
		this->height = height;
	}

	Frame::Frame(unsigned int width, unsigned int height): IFrame(width, height), handle(0)
	{
		glGenFramebuffers(1, &this->handle);
	}

	Frame::~Frame()
	{
		// Also silently ignores 0.
		glDeleteFramebuffers(1, &this->handle);
	}

	void Frame::bind() const
	{
		this->verify();
		glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
		glViewport(0, 0, this->get_width(), this->get_height());
	}

	bool Frame::complete() const
	{
		this->verify();
		this->bind();
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		return status == GL_FRAMEBUFFER_COMPLETE;
	}

	bool Frame::operator==(GLuint handle) const
	{
		return this->handle == handle;
	}

	bool Frame::operator!=(GLuint handle) const
	{
		return this->handle != handle;
	}

	void Frame::verify() const
	{
		topaz_assert(this->handle != 0, "tz::gl::Frame::verify(): Verification Failed!");
	}

	WindowFrame::WindowFrame(GLFWwindow* handle): IFrame(0,0), handle(handle)
	{
		// TODO: Sort out width and height stuff...
	}

	void WindowFrame::bind() const
	{
		topaz_assert(this->handle == glfwGetCurrentContext(), "tz::gl::WindowFrame::bind(): Cannot bind to this WindowFrame because the underlying GLFW context is not current.");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		int w, h;
		glfwGetWindowSize(this->handle, &w, &h);
		glViewport(0, 0, w, h);
	}

	bool WindowFrame::complete() const
	{
		this->bind();
		return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	bool WindowFrame::operator==(GLuint handle) const
	{
		return handle == 0 && (this->handle == glfwGetCurrentContext());
	}

	bool WindowFrame::operator!=(GLuint handle) const
	{
		return handle != 0 || (this->handle != glfwGetCurrentContext());
	}

	namespace bound
	{
		GLuint frame()
		{
			GLint param;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &param);
			return static_cast<GLuint>(param);
		}
	}
}