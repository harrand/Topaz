#include "tz/topaz.hpp"
#include "tz/ren/quad.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"

int main()
{
	tz::initialise();
	tz::os::open_window({});

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::ren::quad_renderer_handle ren = tz_must(tz::ren::create_quad_renderer({}));

	//tz_must(tz::ren::destroy_quad_renderer(ren));
	while(tz::os::window_is_open())
	{
		tz::os::window_update();
		tz::gpu::execute(tz::ren::quad_renderer_graph(ren));
	}

	tz::terminate();
}