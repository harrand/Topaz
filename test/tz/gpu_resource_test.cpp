#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/resource.hpp"

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	std::array<std::byte, 80u * 80u * 4> empty_imgdata;
	tz::gpu::resource_handle img = tz_must(tz::gpu::create_image
	({
		.width = 80u,
		.height = 80u,
		.type = tz::gpu::image_type::rgba,
		.data = empty_imgdata
	}));

	int data = 5;
	tz::gpu::resource_handle buf = tz_must(tz::gpu::create_buffer
	({
		.data = std::as_bytes(std::span<const int>(&data, 1))
	}));

	tz_must(tz::gpu::destroy_resource(buf));

	tz::terminate();

	return 0;
}
