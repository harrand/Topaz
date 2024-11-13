#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"

void go()
{
	// turn on engine. select a gpu. do nothing. turn off engine.
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::terminate();
}

#include "tz/main.hpp"
int tz_main()
{
	// go twice, to test that terminate -> initialise again works.
	go();
	go();
	return 0;
}
