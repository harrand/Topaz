#include "tz/topaz.hpp"
#include "tz/ren/quad.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();
	tz::os::open_window({.flags = tz::os::window_flags::invisible});

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::ren::quad_renderer_handle ren = tz_must(tz::ren::create_quad_renderer({.flags = tz::ren::quad_renderer_flag::graph_present_after}));

	tz::ren::quad_handle quad1 = tz_must(tz::ren::quad_renderer_create_quad(ren, {.position = tz::v2f::zero(), .scale = {0.2f, 0.2f}, .colour = {0.0f, 1.0f, 0.25f}}));

	tz_must(tz::ren::quad_renderer_create_quad(ren, {.position = {-0.5f, -0.5f}, .scale = {0.15f, 0.15f}, .colour = {0.5f, 0.1f, 0.85f}}));

	for(std::size_t i = 0; i < 64; i++)
	{
		tz::os::window_update();
		tz::gpu::execute(tz::ren::quad_renderer_graph(ren));
	}

	tz_must(tz::ren::destroy_quad_renderer(ren));
	tz::terminate();
}