//
// Created by Harrand on 04/01/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/texture.hpp"

tz::test::Case checkerboard_texture()
{
    tz::test::Case test_case("tz::gl::Texture Checkerboard Tests");
    tz::gl::PixelRGBA8 black_pixel{std::byte{}, std::byte{}, std::byte{}, std::byte{255}};
    tz::gl::PixelRGBA8 white_pixel{std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}};
    tz::gl::Image<tz::gl::PixelRGBA8> rgba_checkerboard{2, 2};
    rgba_checkerboard(0, 0) = black_pixel;
    rgba_checkerboard(1, 0) = white_pixel;
    rgba_checkerboard(0, 1) = white_pixel;
    rgba_checkerboard(1, 1) = black_pixel;

    tz::gl::Texture checkerboard;
    // Nothing in here right now.
    topaz_expect(test_case, checkerboard.empty(), "Empty tz::gl::Texture doesn't think it's empty!");
    topaz_expect(test_case, checkerboard.get_width() == checkerboard.get_height() && checkerboard.get_width() == 0, "tz::gl::Texture had unexpected width and height. Expected 0x0, but got ", checkerboard.get_width(), "x", checkerboard.get_height());
    // Send the data off to the GPU!
    checkerboard.set_data(rgba_checkerboard);
    // Now we should have some data!
    topaz_expect(test_case, !checkerboard.empty(), "tz::gl::Texture wrongly thinks it's empty!");
    topaz_expect(test_case, checkerboard.get_width() == checkerboard.get_height() && checkerboard.get_width() == 2, "tz::gl::Texture had unexpected width and height. Expected 2x2 but got ", checkerboard.get_width(), "x", checkerboard.get_height());
    // Get it back and ensure everything is the same.
    tz::gl::Image<tz::gl::PixelRGBA8> retrieved = checkerboard.get_data<tz::gl::PixelRGBA, typename tz::gl::PixelRGBA8::ComponentType>();
    topaz_expect(test_case, retrieved == rgba_checkerboard, "tz::gl::Texture send/retrieve somehow affected the texture data...");
    return test_case;
}

int main()
{
    tz::test::Unit tex;

    // We require topaz to be initialised.
    {
        tz::core::initialise("Texture Tests");
        tex.add(checkerboard_texture());
        tz::core::terminate();
    }
    return tex.result();
}