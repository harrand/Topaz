#include "core/tz.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/image.hpp"
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
	tz::GameInfo game{"ogl_image_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		create_empty_image();
	}
	tz::terminate();
	return 0;
}
