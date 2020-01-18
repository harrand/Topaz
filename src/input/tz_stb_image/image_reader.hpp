#ifndef TOPAZ_EXT_IMAGE_READER_HPP
#define TOPAZ_EXT_IMAGE_READER_HPP
#include "gl/image.hpp"

namespace tz::ext::stb
{
    template<class PixelType>
    tz::gl::Image<PixelType> read_image(const char* path);
}

#include "input/tz_stb_image/image_reader.inl"
#endif // TOPAZ_EXT_IMAGE_READER_HPP