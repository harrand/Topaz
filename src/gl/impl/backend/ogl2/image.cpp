#if TZ_OGL
#include "gl/impl/backend/ogl2/image.hpp"

namespace tz::gl::ogl2
{
	Image::Image(ImageInfo info):
	image(0),
	info(info),
	maybe_bindless_handle(std::nullopt)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &this->image);

		glTextureParameteri(this->image, GL_TEXTURE_WRAP_S, static_cast<GLint>(this->info.sampler.address_mode_s));
		glTextureParameteri(this->image, GL_TEXTURE_WRAP_T, static_cast<GLint>(this->info.sampler.address_mode_t));
		glTextureParameteri(this->image, GL_TEXTURE_WRAP_R, static_cast<GLint>(this->info.sampler.address_mode_r));

		glTextureParameteri(this->image, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(this->info.sampler.min_filter));
		glTextureParameteri(this->image, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(this->info.sampler.mag_filter));

		glTextureStorage2D(this->image, 1, get_format_data(this->info.format).internal_format, static_cast<GLsizei>(this->info.dimensions[0]), static_cast<GLsizei>(this->info.dimensions[1]));
	}

	Image::Image(Image&& move):
	image(0),
	info(),
	maybe_bindless_handle(std::nullopt)
	{
		*this = std::move(move);
	}

	Image::~Image()
	{
		glDeleteTextures(1, &this->image);
	}

	Image& Image::operator=(Image&& rhs)
	{
		std::swap(this->image, rhs.image);
		std::swap(this->info, rhs.info);
		std::swap(this->maybe_bindless_handle, rhs.maybe_bindless_handle);
		return *this;
	}

	ImageFormat Image::get_format() const
	{
		return this->info.format;
	}

	tz::Vec2ui Image::get_dimensions() const
	{
		return this->info.dimensions;
	}

	const Sampler& Image::get_sampler() const
	{
		return this->info.sampler;
	}

	void Image::make_bindless()
	{
		tz_assert(!this->is_bindless(), "Image is being made bindless, but it was already bindless. Please submit a bug report.");
		this->maybe_bindless_handle = glGetTextureHandleARB(this->image);
	}

	bool Image::is_bindless() const
	{
		return this->maybe_bindless_handle.has_value();
	}

	Image::NativeType Image::native() const
	{
		return this->image;
	}

	Image Image::null()
	{
		return {};
	}

	bool Image::is_null() const
	{
		return this->image == 0;
	}

	Image::Image():
	image(0),
	info(),
	maybe_bindless_handle(std::nullopt)
	{}

	namespace image
	{
		void copy(const Image& source, Image& destination)
		{
			tz_assert(source.get_format() == destination.get_format(), "Image Copy - Source and destination must have identical formats.");
			glCopyImageSubData(source.native(), GL_TEXTURE_2D, 0, 0, 0, 0, destination.native(), GL_TEXTURE_2D, 0, 0, 0, 0, source.get_dimensions()[0], source.get_dimensions()[1], 0);
		}

		Image clone_resized(const Image& image, tz::Vec2ui new_size)
		{
			Image newimg
			{{
				.format = image.get_format(),
				.dimensions = new_size,
				.sampler = image.get_sampler()
			}};
			tz::Vec2ui min
			{
				std::min(image.get_dimensions()[0], new_size[0]),
				std::min(image.get_dimensions()[1], new_size[1])
			};
			glCopyImageSubData(image.native(), GL_TEXTURE_2D, 0, 0, 0, 0, newimg.native(), GL_TEXTURE_2D, 0, 0, 0, 0, min[0], min[1], 0);
			if(image.is_bindless())
			{
				newimg.make_bindless();
			}
			return newimg;
		}
	}
}


#endif // TZ_OGL