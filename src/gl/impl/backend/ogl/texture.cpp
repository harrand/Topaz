#if TZ_OGL
#include "gl/impl/backend/ogl/texture.hpp"
#include <utility>

namespace tz::gl::ogl
{
	Texture::Texture(GLsizei width, GLsizei height, Format format, TextureParameters parameters):
	texture(0),
	width(width),
	height(height),
	frontend_format(format),
	format(fmt_internal(this->frontend_format)),
	parameters(parameters)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &this->texture);

		glTextureParameteri(this->texture, GL_TEXTURE_WRAP_S, this->parameters.tex_wrap_s);
		glTextureParameteri(this->texture, GL_TEXTURE_WRAP_T, this->parameters.tex_wrap_t);
		glTextureParameteri(this->texture, GL_TEXTURE_WRAP_R, this->parameters.tex_wrap_u);

		glTextureParameteri(this->texture, GL_TEXTURE_MIN_FILTER, this->parameters.min_filter);
		glTextureParameteri(this->texture, GL_TEXTURE_MAG_FILTER, this->parameters.mag_filter);

		this->resize_and_clear(this->width, this->height);
	}

	Texture::Texture(Texture&& move):
	texture(0),
	width(),
	height(),
	frontend_format(),
	format(),
	parameters()
	{
		*this = std::move(move);
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &this->texture);
	}

	Texture& Texture::operator=(Texture&& rhs)
	{
		std::swap(this->texture, rhs.texture);
		std::swap(this->width, rhs.width);
		std::swap(this->height, rhs.height);
		std::swap(this->frontend_format, rhs.frontend_format);
		std::swap(this->format, rhs.format);
		std::swap(this->parameters, rhs.parameters);
		return *this;
	}

	GLsizei Texture::get_width() const
	{
		return this->width;
	}

	GLsizei Texture::get_height() const
	{
		return this->height;
	}

	Texture::Format Texture::get_format() const
	{
		return this->frontend_format;
	}

	void Texture::resize_and_clear(GLsizei width, GLsizei height)
	{
		this->width = width;
		this->height = height;

		glBindTexture(GL_TEXTURE_2D, this->texture);
		glTexImage2D(GL_TEXTURE_2D, 0, this->format.internal_format, this->width, this->height, 0, this->format.format, this->format.type, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::resize(GLsizei width, GLsizei height)
	{
		std::size_t pixel_size_bytes;
		switch(this->frontend_format)
		{
			case Format::Rgba32Signed:
			[[fallthrough]];
			case Format::Rgba32Unsigned:
			[[fallthrough]];
			case Format::Rgba32sRGB:
			[[fallthrough]];
			case Format::Bgra32UnsignedNorm:
			[[fallthrough]];
			case Format::DepthFloat32:
				pixel_size_bytes = 32;
			break;
			default:
				tz_error("Unrecognised format (OpenGL). NYI?");
			break;
		}
		void* buf = std::malloc(width * height * pixel_size_bytes);
		
		// Retrieve our image data.
		glGetTextureImage(this->texture, 0, this->format.format, this->format.type, width * height * pixel_size_bytes, buf);
		// Do an unsafe resize.
		this->resize_and_clear(width, height);
		
		this->width = width;
		this->height = height;
		this->set_image_data(buf, width * height * pixel_size_bytes);

		std::free(buf);
	}

	void Texture::set_image_data(const void* data, [[maybe_unused]] std::size_t data_size_bytes)
	{
		// TODO: Validation with data_size_bytes. Right now it goes until the texture data is full, which may well crash if bad ptr is provided.
		glBindTexture(GL_TEXTURE_2D, this->texture);
		glTexImage2D(GL_TEXTURE_2D, 0, this->format.internal_format, this->width, this->height, 0, this->format.format, this->format.type, data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::bind_at(GLint location)
	{
		glBindTextureUnit(location, this->texture);
	}

	GLuint Texture::native() const
	{
		return this->texture;
	}
}

#endif // TZ_OGL