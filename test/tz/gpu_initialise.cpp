#include "tz/topaz.hpp"
#include "tz/gpu/device.hpp"

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz::gpu::error_code err = tz::gpu::use_hardware(gpu);

	tz::terminate();

	return 0;
}
