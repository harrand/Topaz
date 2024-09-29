#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/resource.hpp"

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz::error_code err = tz::gpu::use_hardware(gpu);
	tz_assert(err == tz::error_code::success, "{} occurred when trying to use hardware \"{}\"", tz::error_code_name(err), gpu.name);

	auto maybe_res = tz::gpu::create_image
	({
		.access = tz::gpu::resource_access::static_access,
		.width = 80u,
		.height = 80u,
		.type = tz::gpu::image_type::rgba,
		.data = {}
	});
	tz_assert(maybe_res.error_or(tz::error_code::success) == tz::error_code::success, "{} occurred when creating image", tz::error_code_name(maybe_res.error()));

	tz::terminate();

	return 0;
}
