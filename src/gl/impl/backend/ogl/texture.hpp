#ifndef TOPAZ_GL_IMPL_BACKEND_OGL_TEXTURE_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL_TEXTURE_HPP
#if TZ_OGL
#include "core/assert.hpp"
#include "glad/glad.h"
#include <cstdint>

namespace tz::gl::ogl
{
	// For OpenGL, GLenum is documented to be 32 bits.
	// So 3 GLenums together = 96 bits. We have no standard way to represent that many bits in an integral type (a viable underlying type for enum class)
	// We could theoretically use uint128 but that's not guaranteed to exist, and we don't want to lock out all architectures without it.

	// We'll have to use underlying int and support all conversions manually.
	struct InternalTextureFormat
	{
		GLenum format;
		GLenum internal_format;
		GLenum type;
	};

	struct TextureParameters
	{
		GLint min_filter;
		GLint mag_filter;

		GLint tex_wrap_s;
		GLint tex_wrap_t;
		GLint tex_wrap_u;
	};

	constexpr TextureParameters default_parameters()
	{
		return 
		{
			.min_filter = GL_LINEAR,
			.mag_filter = GL_LINEAR,
			.tex_wrap_s = GL_CLAMP_TO_EDGE,
			.tex_wrap_t = GL_CLAMP_TO_EDGE,
			.tex_wrap_u = GL_CLAMP_TO_EDGE
		};
	}

	class Texture
	{
	public:
		enum class Format
		{
			Rgba32Signed,
			Rgba32Unsigned,
			Rgba32sRGB,
			DepthFloat32,
			Bgra32UnsignedNorm,
			Undefined
		};

		Texture(GLsizei width, GLsizei height, Format format, TextureParameters parameters = default_parameters());
		Texture(const Texture& copy) = delete;
		Texture(Texture&& move);
		~Texture();

		Texture& operator=(const Texture& rhs) = delete;
		Texture& operator=(Texture&& rhs);

		GLsizei get_width() const;
		GLsizei get_height() const;
		Format get_format() const;

		void resize_and_clear(GLsizei width, GLsizei height);
		void resize(GLsizei width, GLsizei height);
		void set_image_data(const void* data, std::size_t data_size_bytes);
		void bind_at(GLint location);

		GLuint native() const;
	private:
		GLuint texture;
		GLsizei width;
		GLsizei height;
		Format frontend_format;
		InternalTextureFormat format;
		TextureParameters parameters;
	};

	constexpr InternalTextureFormat fmt_internal(Texture::Format fmt)
	{
		switch(fmt)
		{
			case Texture::Format::Rgba32Signed:
				return
				{
					.format = GL_RGBA_INTEGER,
					.internal_format = GL_RGBA8I,
					.type = GL_INT
				};
			break;
			case Texture::Format::Rgba32Unsigned:
				return
				{
					.format = GL_RGBA_INTEGER,
					.internal_format = GL_RGBA8UI,
					.type = GL_UNSIGNED_INT
				};
			break;
			case Texture::Format::Rgba32sRGB:
				return
				{
					.format = GL_RGBA,
					.internal_format = GL_SRGB8_ALPHA8,
					.type = GL_UNSIGNED_BYTE
				};
			break;
			case Texture::Format::DepthFloat32:
				return
				{
					.format = GL_DEPTH_COMPONENT,
					.internal_format = GL_DEPTH_COMPONENT32F,
					.type = GL_FLOAT
				};
			break;
			case Texture::Format::Bgra32UnsignedNorm:
				return
				{
					.format = GL_BGRA,
					.internal_format = GL_RGBA8,
					.type = GL_UNSIGNED_BYTE
				};
			break;
			default:
				tz_error("Unrecognised OpenGL Format (Failed to retrieve internal texture format)");
				return {};
			break;
		}
	}
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL_TEXTURE_HPP