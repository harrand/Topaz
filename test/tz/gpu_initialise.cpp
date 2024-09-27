#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"

#include "tz/os/window.hpp"

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz::error_code err = tz::gpu::use_hardware(gpu);
	tz_assert(err == tz::error_code::success, "{} occurred when trying to use hardware \"{}\"", tz::error_code_name(err), gpu.name);

	tz::terminate();

	return 0;
}
