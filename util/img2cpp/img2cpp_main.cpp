#include "codegen.hpp"
#include "gl/tz_stb_image/image_reader.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    topaz_assert(argc > 1, "Invalid args");
    const char* image_relative_path = argv[1];
    tz::gl::Image<tz::gl::PixelRGBA8> img = tz::ext::stb::read_image<tz::gl::PixelRGBA8>(image_relative_path);
    std::cout << img2cpp::serialise_cpp(img);
}