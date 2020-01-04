//
// Created by Harrand on 04/01/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/image.hpp"

tz::test::Case checkerboard()
{
    tz::test::Case test_case("tz::gl::Image Checkerboard Tests");
    tz::gl::PixelRGBA8 black_pixel{std::byte{}, std::byte{}, std::byte{}, std::byte{255}};
    tz::gl::PixelRGBA8 white_pixel{std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}};
    tz::gl::Image<tz::gl::PixelRGBA8> rgba_checkerboard{2, 2};
    rgba_checkerboard(0, 0) = black_pixel;
    rgba_checkerboard(1, 0) = white_pixel;
    rgba_checkerboard(0, 1) = white_pixel;
    rgba_checkerboard(1, 1) = black_pixel;

    topaz_expect(test_case, black_pixel != white_pixel, "Can't tell the difference between black and white pixel.");

    auto* data = rgba_checkerboard.data();
    topaz_expect(test_case, *(data + 0) == black_pixel, "Pixel mismatch");
    topaz_expect(test_case, *(data + 1) == white_pixel, "Pixel mismatch");
    topaz_expect(test_case, *(data + 2) == white_pixel, "Pixel mismatch");
    topaz_expect(test_case, *(data + 3) == black_pixel, "Pixel mismatch");
    return test_case;
}

int main()
{
    tz::test::Unit img;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Image Tests");
        img.add(checkerboard());
        tz::core::terminate();
    }
    return img.result();
}