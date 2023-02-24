#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_RENDERBUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_RENDERBUFFER_HPP
#if TZ_OGL
#include "tz/core/data/vector.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/image_format.hpp"

namespace tz::gl::ogl2
{
	struct render_buffer_info
	{
		/// Format of the image data.
		image_format format;
		/// {Width, Height} of the image, in pixels.
		tz::vec2ui dimensions;
	};

	class render_buffer
	{
	public:
		render_buffer(render_buffer_info info);
		render_buffer(const render_buffer& copy) = delete;
		render_buffer(render_buffer&& move);
		~render_buffer();

		render_buffer& operator=(const render_buffer& rhs) = delete;
		render_buffer& operator=(render_buffer&& rhs);

		image_format get_format() const;
		tz::vec2ui get_dimensions() const;
		using NativeType = GLuint;
		NativeType native() const;

		static render_buffer null();
		bool is_null() const;
	private:
		render_buffer();

		GLuint renderbuffer;
		render_buffer_info info;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_RENDERBUFFER_HPP
