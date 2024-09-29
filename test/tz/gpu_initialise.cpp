#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"

void go()
{
	// turn on engine. select a gpu. do nothing. turn off engine.
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz::error_code err = tz::gpu::use_hardware(gpu);
	tz_assert(err == tz::error_code::success, "{} occurred when trying to use hardware \"{}\"", tz::error_code_name(err), gpu.name);

	tz::terminate();
}

int main()
{
	// go twice, to test that terminate -> initialise again works.
	go();
	go();
	return 0;
}
