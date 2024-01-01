#if TZ_OGL
#include "tz/gl/impl/opengl/detail/renderbuffer.hpp"

namespace tz::gl::ogl2
{
	render_buffer::render_buffer(render_buffer_info info):
	renderbuffer(0),
	info(info)
	{
		glCreateRenderbuffers(1, &this->renderbuffer);
		glNamedRenderbufferStorage(this->renderbuffer, get_format_data(this->info.format).internal_format, this->get_dimensions()[0], this->get_dimensions()[1]);
	}

	render_buffer::render_buffer(render_buffer&& move):
	renderbuffer(0),
	info()
	{
		*this = std::move(move);
	}

	render_buffer::~render_buffer()
	{
		glDeleteRenderbuffers(1, &this->renderbuffer);
	}

	render_buffer& render_buffer::operator=(render_buffer&& rhs)
	{
		std::swap(this->renderbuffer, rhs.renderbuffer);
		std::swap(this->info, rhs.info);
		return *this;
	}

	image_format render_buffer::get_format() const
	{
		return this->info.format;
	}

	tz::vec2ui render_buffer::get_dimensions() const
	{
		return this->info.dimensions;
	}

	render_buffer::NativeType render_buffer::native() const
	{
		return this->renderbuffer;
	}

	render_buffer render_buffer::null()
	{
		return {};
	}

	bool render_buffer::is_null() const
	{
		return this->renderbuffer == 0;
	}

	render_buffer::render_buffer():
	renderbuffer(0),
	info{.format = image_format::undefined, .dimensions = {1u, 1u}}{}
}

#endif // TZ_OGL
