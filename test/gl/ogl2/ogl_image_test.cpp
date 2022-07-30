#include "tz/core/tz.hpp"
#include "tz/gl/impl/backend/ogl2/tz_opengl.hpp"
#include "tz/gl/impl/backend/ogl2/image.hpp"
#include <numeric>

void create_empty_image()
{
	using namespace tz::gl::ogl2;
	Image image
	{{
		.format = ImageFormat::RGBA32,
		.dimensions = {1u, 1u},
		.sampler =
		{
			.min_filter = LookupFilter::Nearest,
			.mag_filter = LookupFilter::Nearest,
			.address_mode_s = AddressMode::ClampToEdge,
			.address_mode_t = AddressMode::ClampToEdge,
			.address_mode_r = AddressMode::ClampToEdge
		}
	}};
}

int main()
{
	tz::initialise
	({
		.name = "ogl_image_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		create_empty_image();
	}
	tz::terminate();
	return 0;
}
