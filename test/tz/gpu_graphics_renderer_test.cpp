#include "tz/topaz.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/pass.hpp"
#include "tz/gpu/graph.hpp"
#include "tz/gpu/shader.hpp"

#include ImportedShaderHeader(triangle, vertex)
#include ImportedShaderHeader(triangle, fragment)

int main()
{
	tz::initialise();
	tz::os::open_window({.name = "Graphics Render Test"});

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::gpu::shader_handle graphics = tz_must(tz::gpu::create_graphics_shader(ImportedShaderSource(triangle, vertex), ImportedShaderSource(triangle, fragment)));

	tz::gpu::resource_handle colour_targets[] =
	{
		tz::gpu::window_resource,
	};

	tz::gpu::pass_handle pass = tz_must(tz::gpu::create_pass
	({
		.graphics =
		{
			.clear_colour = {0.0f, 0.0f, 0.0f, 1.0f},
			.colour_targets = colour_targets,
			.flags = tz::gpu::graphics_flag::no_depth_test
		},
		.shader = graphics,
	}));

	tz::gpu::graph_handle graph = tz_must(tz::gpu::graph_builder{}
		.add_pass(pass)
		.build());

	while(tz::os::window_is_open())
	{
		tz::os::window_update();
		tz::gpu::execute(graph);
	}

	tz::gpu::destroy_pass(pass);
	tz::terminate();
	return 0;
}