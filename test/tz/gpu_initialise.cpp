#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz::gpu::error_code err = tz::gpu::use_hardware(gpu);
	tz_assert(err == tz::gpu::error_code::success, "failed to use hardware \"{}\"", gpu.name);

	tz::terminate();

	return 0;
}
