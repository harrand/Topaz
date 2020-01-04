#ifndef TOPAZ_GL_IMAGE_HPP
#define TOPAZ_GL_IMAGE_HPP
#include "gl/pixel.hpp"
#include <vector>

namespace tz::gl
{
    template<class PixelType>
    class Image
    {
    public:
        Image(unsigned int width, unsigned int height);
        unsigned int get_width() const;
        unsigned int get_height() const;
        GLenum get_pixel_type() const;
        const PixelType* data() const;
        PixelType* data();
        const PixelType& operator()(unsigned int x, unsigned int y) const;
        PixelType& operator()(unsigned int x, unsigned int y);
        bool operator==(const Image<PixelType>& rhs) const;
        bool operator!=(const Image<PixelType>& rhs) const;
    private:
        unsigned int width;
        unsigned int height;
        // Represented similarly to a row-major matrix.
        std::vector<PixelType> pixel_data;
    };
}

#include "gl/image.inl"
#endif // TOPAZ_GL_IMAGE_HPP