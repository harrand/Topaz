#include "tz/gl/impl/backend/ogl2/renderbuffer.hpp"
#if TZ_OGL

namespace tz::gl::ogl2
{
	Renderbuffer::Renderbuffer(RenderbufferInfo info):
	renderbuffer(0),
	info(info)
	{
		glCreateRenderbuffers(1, &this->renderbuffer);
		glNamedRenderbufferStorage(this->renderbuffer, get_format_data(this->info.format).internal_format, this->get_dimensions()[0], this->get_dimensions()[1]);
	}

	Renderbuffer::Renderbuffer(Renderbuffer&& move):
	renderbuffer(0),
	info()
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
		std::swap(this->info, rhs.info);
		return *this;
	}

	ImageFormat Renderbuffer::get_format() const
	{
		return this->info.format;
	}

	tz::Vec2ui Renderbuffer::get_dimensions() const
	{
		return this->info.dimensions;
	}

	Renderbuffer::NativeType Renderbuffer::native() const
	{
		return this->renderbuffer;
	}

	Renderbuffer Renderbuffer::null()
	{
		return {};
	}

	bool Renderbuffer::is_null() const
	{
		return this->renderbuffer == 0;
	}

	Renderbuffer::Renderbuffer():
	renderbuffer(0),
	info{.format = ImageFormat::Undefined, .dimensions = {1u, 1u}}{}
}

#endif // TZ_OGL
