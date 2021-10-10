#if TZ_OGL
#include "gl/impl/backend/ogl/renderbuffer.hpp"

namespace tz::gl::ogl
{
	Renderbuffer::Renderbuffer(GLsizei width, GLsizei height, Texture::Format format):
	renderbuffer(),
	width(width),
	height(height),
	format(format)
	{
		glCreateRenderbuffers(1, &this->renderbuffer);
		glNamedRenderbufferStorage(this->renderbuffer, fmt_internal(this->format).internal_format, this->width, this->height);
	}

	Renderbuffer::Renderbuffer(Renderbuffer&& move):
	renderbuffer(0)
	{
		*this = std::move(move);
	}

	Renderbuffer::~Renderbuffer()
	{
		glDeleteRenderbuffers(1, &this->renderbuffer);
	}

	Renderbuffer& Renderbuffer::operator=(Renderbuffer&& rhs)
	{
		std::swap(this->renderbuffer, rhs.renderbuffer);
		std::swap(this->width, rhs.width);
		std::swap(this->height, rhs.height);
		std::swap(this->format, rhs.format);
		return *this;
	}

	GLuint Renderbuffer::native() const
	{
		return this->renderbuffer;
	}
}

#endif // TZ_OGL