
#include "tz/topaz.hpp"
#include "tz/ren/quad.hpp"
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include "tz/gpu/hardware.hpp"

int main()
{
	tz::initialise();
	tz::os::open_window({});

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::ren::quad_renderer_handle ren = tz_must(tz::ren::create_quad_renderer({}));

	tz::ren::quad_handle quad1 = tz_must(tz::ren::quad_renderer_create_quad(ren, {.position = tz::v2f::zero(), .scale = {0.2f, 0.2f}, .colour = {0.0f, 1.0f, 0.25f}}));

	tz_must(tz::ren::quad_renderer_create_quad(ren, {.position = {-0.5f, -0.5f}, .scale = {0.15f, 0.15f}, .colour = {0.5f, 0.1f, 0.85f}}));

	while(tz::os::window_is_open())
	{
		tz::os::window_update();
		tz::gpu::execute(tz::ren::quad_renderer_graph(ren));
		tz::v2f pos = tz::ren::get_quad_position(ren, quad1);
		tz::v2f scale = tz::ren::get_quad_scale(ren, quad1);
		if(tz::os::is_key_pressed(tz::os::key::a))
		{
			tz::ren::set_quad_position(ren, quad1, pos += {-0.002f, 0.0f});
		}
		if(tz::os::is_key_pressed(tz::os::key::d))
		{
			tz::ren::set_quad_position(ren, quad1, pos += {0.002f, 0.0f});
		}
		if(tz::os::is_key_pressed(tz::os::key::w))
		{
			tz::ren::set_quad_position(ren, quad1, pos += {0.0f, 0.002f});
		}
		if(tz::os::is_key_pressed(tz::os::key::s))
		{
			tz::ren::set_quad_position(ren, quad1, pos += {0.0f, -0.002f});
		}

		if(tz::os::is_key_pressed(tz::os::key::i))
		{
			tz::ren::set_quad_scale(ren, quad1, scale * 1.001f);
		}
		if(tz::os::is_key_pressed(tz::os::key::k))
		{
			tz::ren::set_quad_scale(ren, quad1, scale * 0.999f);
		}
	}

	tz_must(tz::ren::destroy_quad_renderer(ren));
	tz::terminate();
}