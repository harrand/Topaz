#ifndef TOPAZ_UTIL_IMG2CPP_CODEGEN_HPP
#define TOPAZ_UTIL_IMG2CPP_CODEGEN_HPP
#include "gl/image.hpp"
#include <string>

namespace img2cpp
{
    template<tz::gl::PixelType PixelT>
    constexpr std::string serialise_cpp(const tz::gl::Image<PixelT>& image);
}

#include "codegen.inl"
#endif // TOPAZ_UTIL_IMG2CPP_CODEGEN_HPP