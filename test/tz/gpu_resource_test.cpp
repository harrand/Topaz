#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/resource.hpp"

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz::error_code err = tz::gpu::use_hardware(gpu);
	tz_assert(err == tz::error_code::success, "{} occurred when trying to use hardware \"{}\"", tz::error_code_name(err), gpu.name);

	tz::gpu::resource_handle img = tz_must(tz::gpu::create_image
	({
		.access = tz::gpu::resource_access::static_access,
		.width = 80u,
		.height = 80u,
		.type = tz::gpu::image_type::rgba,
		.data = {}
	}));

	int data = 5;
	tz::gpu::resource_handle buf = tz_must(tz::gpu::create_buffer
	({
		.access = tz::gpu::resource_access::dynamic_access,
		.type = tz::gpu::buffer_type::storage,
		.data = std::as_bytes(std::span<const int>(&data, 1))
	}));

	tz_must(tz::gpu::destroy_resource(buf));

	tz::terminate();

	return 0;
}
