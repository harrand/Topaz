#ifndef TOPAZ_EXT_IMAGE_WRITER_HPP
#define TOPAZ_EXT_IMAGE_WRITER_HPP
#include "gl/image.hpp"
#include <string>

namespace tz::ext::stb
{
	template<class PixelType>
	std::string write_image(const char* path, const tz::gl::Image<PixelType>& image, std::string name = "tz_img.png");
}

#include "ext/tz_stb_image/image_writer.inl"
#endif