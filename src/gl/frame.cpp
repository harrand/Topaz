#include "gl/frame.hpp"
#include "core/debug/assert.hpp"
#include "GLFW/glfw3.h"

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

	void IFrame::clear() const
	{
		topaz_assert(this->operator==(tz::gl::bound::frame()), "tz::gl::IFrame::clear(): This frame must be bound before clearing.");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void IFrame::set_clear_color(float r, float g, float b)
	{
		topaz_assert(this->operator==(tz::gl::bound::frame()), "tz::gl::IFrame::clear(): This frame must be bound before clearing.");
		glClearColor(r, g, b, 1.0f);
	}

	Frame::Frame(unsigned int width, unsigned int height): IFrame(width, height), handle(0), attachments(), pending_attachments()
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
		this->process_pending_attachments();
		//topaz_assert(this->complete(), "tz::gl::Frame::bind(): Attempting to bind an incomplete frame. This means something has been missed in its setup process.");
		glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
		glViewport(0, 0, this->get_width(), this->get_height());
	}

	bool Frame::complete() const
	{
		this->verify();
		glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return status == GL_FRAMEBUFFER_COMPLETE;
	}

	void Frame::set_output_attachment(GLenum attachment) const
	{
		GLenum attachments[] = {attachment};
		glNamedFramebufferDrawBuffers(this->handle, 1, attachments);
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

	void Frame::process_pending_attachments() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
		while(!pending_attachments.empty())
		{
			GLenum attachment = pending_attachments.front().first;
			tz::gl::Texture* tex = pending_attachments.front().second;
			tex->bind_to_frame(attachment);
			pending_attachments.pop();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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