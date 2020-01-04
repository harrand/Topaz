#include "core/debug/assert.hpp"

namespace tz::gl
{
    template<class PixelType>
    Image<PixelType>::Image(unsigned int width, unsigned int height): width(width), height(height), pixel_data(this->width * this->height, PixelType{})
    {
        static_assert(tz::gl::pixel::parse_component_type<typename PixelType::ComponentType>() != GL_INVALID_VALUE, "Invalid pixel component type detected.");
    }

    template<class PixelType>
    unsigned int Image<PixelType>::get_width() const
    {
        return this->width;
    }

    template<class PixelType>
    unsigned int Image<PixelType>::get_height() const
    {
        return this->height;
    }

    template<class PixelType>
    GLenum Image<PixelType>::get_pixel_type() const
    {
        return tz::gl::pixel::parse_component_type<typename PixelType::ComponentType>();
    }

    template<class PixelType>
    const PixelType* Image<PixelType>::data() const
    {
        return this->pixel_data.data();
    }

    template<class PixelType>
    PixelType* Image<PixelType>::data()
    {
        return this->pixel_data.data();
    }

    template<class PixelType>
    const PixelType& Image<PixelType>::operator()(unsigned int x, unsigned int y) const
    {
        topaz_assert(x < this->width && y < this->height, "Image<PixelType>::operator[](", x, ", ", y, "): x and y values out-of-range. Size: (", this->width, ", ", this->height, ")");
        return this->pixel_data[(this->width * y) + x];
    }

    template<class PixelType>
    PixelType& Image<PixelType>::operator()(unsigned int x, unsigned int y)
    {
        topaz_assert(x < this->width && y < this->height, "Image<PixelType>::operator[](", x, ", ", y, "): x and y values out-of-range. Size: (", this->width, ", ", this->height, ")");
        return this->pixel_data[(this->width * y) + x];
    }

    template<class PixelType>
    bool Image<PixelType>::operator==(const Image<PixelType>& rhs) const
    {
        return this->width == rhs.width && this->height == rhs.height && this->pixel_data == rhs.pixel_data;
    }

    template<class PixelType>
    bool Image<PixelType>::operator!=(const Image<PixelType>& rhs) const
    {
        return this->width != rhs.width || this->height != rhs.height || this->pixel_data != rhs.pixel_data;
    }
}