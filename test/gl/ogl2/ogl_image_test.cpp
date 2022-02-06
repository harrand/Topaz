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

void create_empty_bindless_image()
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
	tz_assert(!image.is_bindless(), "Image was wrongly considered to be bindless when it really shouldn't (this is a major hazard, fix this asap)");
	image.make_bindless();
	tz_assert(image.is_bindless(), "Image was wrongly considered NOT to be bindless when it really should be.");
}

int main()
{
	tz::initialise
	({
		.name = "ogl_image_test",
		.app_type = tz::ApplicationType::HiddenWindowApplication
	});
	{
		create_empty_image();
		create_empty_bindless_image();
	}
	tz::terminate();
	return 0;
}
