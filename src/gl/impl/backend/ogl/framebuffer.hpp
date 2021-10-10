#ifndef TOPAZ_GL_IMPL_BACKEND_OGL_FRAMEBUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL_FRAMEBUFFER_HPP
#if TZ_OGL
#include "gl/impl/backend/ogl/renderbuffer.hpp"
#include "gl/impl/backend/ogl/texture.hpp"
#include <deque>
#include <utility>
#include <variant>

namespace tz::gl::ogl
{
	class Framebuffer
	{
	public:
		Framebuffer();
		Framebuffer(const Framebuffer& copy) = delete;
		Framebuffer(Framebuffer&& move);
		~Framebuffer();

		Framebuffer& operator=(const Framebuffer& rhs) = delete;
		Framebuffer& operator=(Framebuffer&& rhs);

		void attach(GLenum attachment, const Texture& texture);
		void attach(GLenum attachment, const Renderbuffer& renderbuffer);

		void set_output(GLenum attachment);
		void bind() const;
	private:
		using TextureVariant = std::variant<const Texture*, const Renderbuffer*>;

		bool complete() const;

		GLuint framebuffer;
		std::deque<std::pair<GLenum, TextureVariant>> attachments;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL_FRAMEBUFFER_HPP