//
// Created by Harrand on 04/01/2020.
//

#include "test_framework.hpp"
#include "core/tz.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "gl/texture.hpp"
#include "algo/static.hpp"

TZ_TEST_BEGIN(statics)
	topaz_expect(!tz::algo::copyable<tz::gl::Texture>(), "tz::gl::Texture is copyable. This is wrong.");
	topaz_expect(tz::algo::moveable<tz::gl::Texture>(), "tz::gl::Texture is not moveable. This is wrong.");
TZ_TEST_END

TZ_TEST_BEGIN(checkerboard_texture)
	tz::gl::PixelRGBA8 black_pixel{std::byte{}, std::byte{}, std::byte{}, std::byte{255}};
	tz::gl::PixelRGBA8 white_pixel{std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}};
	tz::gl::Image<tz::gl::PixelRGBA<std::byte>> rgba_checkerboard{2, 2};
	rgba_checkerboard(0, 0) = black_pixel;
	rgba_checkerboard(1, 0) = white_pixel;
	rgba_checkerboard(0, 1) = white_pixel;
	rgba_checkerboard(1, 1) = black_pixel;

	tz::gl::Texture checkerboard;
	// Nothing in here right now.
	topaz_expect(checkerboard.empty(), "Empty tz::gl::Texture doesn't think it's empty!");
	topaz_expect(checkerboard.get_width() == checkerboard.get_height() && checkerboard.get_width() == 0, "tz::gl::Texture had unexpected width and height. Expected 0x0, but got ", checkerboard.get_width(), "x", checkerboard.get_height());
	// Send the data off to the GPU!
	checkerboard.set_data(rgba_checkerboard);
	// Now we should have some data!
	topaz_expect(!checkerboard.empty(), "tz::gl::Texture wrongly thinks it's empty!");
	topaz_expect(checkerboard.get_width() == checkerboard.get_height() && checkerboard.get_width() == 2, "tz::gl::Texture had unexpected width and height. Expected 2x2 but got ", checkerboard.get_width(), "x", checkerboard.get_height());
	// Get it back and ensure everything is the same.
	tz::gl::Image<tz::gl::PixelRGBA8> retrieved = checkerboard.get_data<tz::gl::PixelRGBA8>();
	topaz_expect(retrieved == rgba_checkerboard, "tz::gl::Texture send/retrieve somehow affected the texture data...");
TZ_TEST_END

int main()
{
	tz::test::Unit tex;

	// We require topaz to be initialised.
	{
		tz::initialise("Texture Tests", tz::invisible_tag);
		tex.add(statics());
		tex.add(checkerboard_texture());
		tz::terminate();
	}
	return tex.result();
}