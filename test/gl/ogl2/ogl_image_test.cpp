#include "tz/tz.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/image.hpp"
#include <numeric>

void create_empty_image()
{
	using namespace tz::gl::ogl2;
	image img
	{{
		.format = image_format::RGBA32,
		.dimensions = {1u, 1u},
		.shader_sampler =
		{
			.min_filter = lookup_filter::nearest,
			.mag_filter = lookup_filter::nearest,
			.address_mode_s = address_mode::clamp_to_edge,
			.address_mode_t = address_mode::clamp_to_edge,
			.address_mode_r = address_mode::clamp_to_edge
		}
	}};
}

int main()
{
	tz::initialise
	({
		.name = "ogl_image_test",
		.flags = {tz::application_flag::window_hidden}
	});
	{
		create_empty_image();
	}
	tz::terminate();
	return 0;
}
