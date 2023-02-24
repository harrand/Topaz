#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/gl/impl/opengl/detail/image.hpp"

namespace tz::gl::ogl2
{
	image::image(image_info info):
	img(0),
	info(info),
	maybe_bindless_handle(std::nullopt)
	{
		TZ_PROFZONE("ogl - image create", 0xFFAA0000);
		glCreateTextures(GL_TEXTURE_2D, 1, &this->img);

		glTextureParameteri(this->img, GL_TEXTURE_WRAP_S, static_cast<GLint>(this->info.shader_sampler.address_mode_s));
		glTextureParameteri(this->img, GL_TEXTURE_WRAP_T, static_cast<GLint>(this->info.shader_sampler.address_mode_t));
		glTextureParameteri(this->img, GL_TEXTURE_WRAP_R, static_cast<GLint>(this->info.shader_sampler.address_mode_r));

		glTextureParameteri(this->img, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(this->info.shader_sampler.min_filter));
		glTextureParameteri(this->img, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(this->info.shader_sampler.mag_filter));

		glTextureStorage2D(this->img, 1, get_format_data(this->info.format).internal_format, static_cast<GLsizei>(this->info.dimensions[0]), static_cast<GLsizei>(this->info.dimensions[1]));
	}

	image::image(image&& move):
	img(0),
	info(),
	maybe_bindless_handle(std::nullopt)
	{
		*this = std::move(move);
	}

	image::~image()
	{
		if(this->maybe_bindless_handle.has_value())
		{
			glMakeTextureHandleNonResidentARB(this->maybe_bindless_handle.value());
			this->maybe_bindless_handle = std::nullopt;
		}
		glDeleteTextures(1, &this->img);
	}

	image& image::operator=(image&& rhs)
	{
		std::swap(this->img, rhs.img);
		std::swap(this->info, rhs.info);
		std::swap(this->maybe_bindless_handle, rhs.maybe_bindless_handle);
		return *this;
	}

	image_format image::get_format() const
	{
		return this->info.format;
	}

	tz::vec2ui image::get_dimensions() const
	{
		return this->info.dimensions;
	}

	const sampler& image::get_sampler() const
	{
		return this->info.shader_sampler;
	}

	void image::set_data(std::span<const std::byte> texture_data)
	{
		TZ_PROFZONE("ogl - image set data", 0xFFAA0000);
		const FormatData internal_fmt = get_format_data(this->get_format());
		glTextureSubImage2D(this->img, 0, 0, 0, this->get_dimensions()[0], this->get_dimensions()[1], internal_fmt.format, internal_fmt.type, texture_data.data());
	}

	void image::make_bindless()
	{
		TZ_PROFZONE("ogl - image make bindless", 0xFFAA0000);
		tz::assert(supports_bindless_textures(), "Attempted to make an image bindless, but the bindless textures extension (\"GL_ARB_bindless_texture\") is not available on this machine. Your hardware/drivers do not support this specific OGL backend.");
		tz::assert(!this->is_bindless(), "image is being made bindless, but it was already bindless. Please submit a bug report.");
		this->maybe_bindless_handle = glGetTextureHandleARB(this->img);
		glMakeTextureHandleResidentARB(this->maybe_bindless_handle.value());
	}

	bool image::is_bindless() const
	{
		return this->maybe_bindless_handle.has_value();
	}

	image::bindless_handle image::get_bindless_handle() const
	{
		tz::assert(this->is_bindless(), "Attempted to retrieve bindless handle for image which is not bindless.");
		return this->maybe_bindless_handle.value();
	}

	image::NativeType image::native() const
	{
		return this->img;
	}

	std::string image::debug_get_name() const
	{
		return this->debug_name;
	}

	void image::debug_set_name(std::string name)
	{
		this->debug_name = name;
		#if TZ_DEBUG
			glObjectLabel(GL_TEXTURE, this->img, -1, this->debug_name.c_str());
		#endif
	}

	image image::null()
	{
		return {};
	}

	bool image::is_null() const
	{
		return this->img == 0;
	}

	image::image():
	img(0),
	info(),
	maybe_bindless_handle(std::nullopt)
	{}

	namespace image_helper
	{
		void copy(const image& source, image& destination)
		{
			TZ_PROFZONE("ogl - image copy operation", 0xFFAA0000);
			tz::assert(source.get_format() == destination.get_format(), "image Copy - Source and destination must have identical formats.");
			glCopyImageSubData(source.native(), GL_TEXTURE_2D, 0, 0, 0, 0, destination.native(), GL_TEXTURE_2D, 0, 0, 0, 0, source.get_dimensions()[0], source.get_dimensions()[1], 1);
		}

		image clone_resized(const image& img, tz::vec2ui new_size)
		{
			TZ_PROFZONE("ogl - image clone operation", 0xFFAA0000);
			image newimg
			{{
				.format = img.get_format(),
				.dimensions = new_size,
				.shader_sampler = img.get_sampler()
			}};
			tz::vec2ui min
			{
				std::min(img.get_dimensions()[0], new_size[0]),
				std::min(img.get_dimensions()[1], new_size[1])
			};
			glCopyImageSubData(img.native(), GL_TEXTURE_2D, 0, 0, 0, 0, newimg.native(), GL_TEXTURE_2D, 0, 0, 0, 0, min[0], min[1], 1);
			if(img.is_bindless())
			{
				newimg.make_bindless();
			}
			return newimg;
		}
	}
}


#endif // TZ_OGL
