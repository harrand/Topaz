#include "core/debug/assert.hpp"

namespace tz::gl
{
	template<PixelType PixelT>
	Image<PixelT>::Image(unsigned int width, unsigned int height): width(width), height(height), pixel_data(this->width * this->height, PixelT{})
	{
		static_assert(tz::gl::pixel::parse_component_type<typename PixelT::ComponentType>() != GL_INVALID_VALUE, "Invalid pixel component type detected.");
	}

	template<PixelType PixelT>
	unsigned int Image<PixelT>::get_width() const
	{
		return this->width;
	}

	template<PixelType PixelT>
	unsigned int Image<PixelT>::get_height() const
	{
		return this->height;
	}

	template<PixelType PixelT>
	GLenum Image<PixelT>::get_pixel_type() const
	{
		return tz::gl::pixel::parse_component_type<typename PixelT::ComponentType>();
	}

	template<PixelType PixelT>
	const PixelT* Image<PixelT>::data() const
	{
		return this->pixel_data.data();
	}

	template<PixelType PixelT>
	PixelT* Image<PixelT>::data()
	{
		return this->pixel_data.data();
	}

	template<PixelType PixelT>
	const PixelT& Image<PixelT>::operator()(unsigned int x, unsigned int y) const
	{
		topaz_assert(x < this->width && y < this->height, "Image<PixelT>::operator[](", x, ", ", y, "): x and y values out-of-range. Size: (", this->width, ", ", this->height, ")");
		return this->pixel_data[(this->width * y) + x];
	}

	template<PixelType PixelT>
	PixelT& Image<PixelT>::operator()(unsigned int x, unsigned int y)
	{
		topaz_assert(x < this->width && y < this->height, "Image<PixelT>::operator[](", x, ", ", y, "): x and y values out-of-range. Size: (", this->width, ", ", this->height, ")");
		return this->pixel_data[(this->width * y) + x];
	}

	template<PixelType PixelT>
	bool Image<PixelT>::operator==(const Image<PixelT>& rhs) const
	{
		return this->width == rhs.width && this->height == rhs.height && this->pixel_data == rhs.pixel_data;
	}

	template<PixelType PixelT>
	bool Image<PixelT>::operator!=(const Image<PixelT>& rhs) const
	{
		return this->width != rhs.width || this->height != rhs.height || this->pixel_data != rhs.pixel_data;
	}
}