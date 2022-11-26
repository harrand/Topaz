#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_RENDERBUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_RENDERBUFFER_HPP
#if TZ_OGL
#include "hdk/data/vector.hpp"
#include "tz/gl/impl/backend/ogl2/tz_opengl.hpp"
#include "tz/gl/impl/backend/ogl2/image_format.hpp"

namespace tz::gl::ogl2
{
	struct RenderbufferInfo
	{
		/// Format of the image data.
		ImageFormat format;
		/// {Width, Height} of the image, in pixels.
		hdk::vec2ui dimensions;
	};

	class Renderbuffer
	{
	public:
		Renderbuffer(RenderbufferInfo info);
		Renderbuffer(const Renderbuffer& copy) = delete;
		Renderbuffer(Renderbuffer&& move);
		~Renderbuffer();

		Renderbuffer& operator=(const Renderbuffer& rhs) = delete;
		Renderbuffer& operator=(Renderbuffer&& rhs);

		ImageFormat get_format() const;
		hdk::vec2ui get_dimensions() const;
		using NativeType = GLuint;
		NativeType native() const;

		static Renderbuffer null();
		bool is_null() const;
	private:
		Renderbuffer();

		GLuint renderbuffer;
		RenderbufferInfo info;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_RENDERBUFFER_HPP
