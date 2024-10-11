#include "tz/topaz.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/pass.hpp"
#include "tz/gpu/graph.hpp"
#include ImportedTextHeader(empty_vertex, spv)
#include ImportedTextHeader(empty_fragment, spv)

int main()
{
	tz::initialise();
	tz::os::open_window({.name = "Graphics Render Test"});

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::gpu::shader_handle graphics = tz_must(tz::gpu::create_graphics_shader(ImportedTextData(empty_vertex, spv), ImportedTextData(empty_fragment, spv)));

	tz::gpu::resource_handle colour_targets[] =
	{
		tz::gpu::window_resource,
		tz_must(tz::gpu::create_image
		({
			.access = tz::gpu::resource_access::static_access,
			.width = tz::os::window_get_width(),
			.height = tz::os::window_get_height(),
			.type = tz::gpu::image_type::rgba,
			.data = {},
			.flags = tz::gpu::image_flag::colour_target
		})),
	};

	tz::gpu::resource_handle depth_target = tz_must(tz::gpu::create_image
	({
		.access = tz::gpu::resource_access::static_access,
		.width = tz::os::window_get_width(),
		.height = tz::os::window_get_height(),
		.type = tz::gpu::image_type::depth,
		.data = {},
		.flags = tz::gpu::image_flag::depth_target
	}));

	tz::gpu::pass_handle pass = tz_must(tz::gpu::create_pass
	({
		.graphics =
		{
			.clear_colour = {0.0f, 0.0f, 0.0f},
			.colour_targets = colour_targets,
			.depth_target = depth_target,
		},
		.shader = graphics,
	}));

	tz::gpu::graph_handle graph = tz_must(tz::gpu::create_graph
	({
		.timeline = {&pass, 1}
	}));

	while(tz::os::window_is_open())
	{
		tz::os::window_update();
		tz::gpu::execute(graph);
	}

	tz::gpu::destroy_pass(pass);
	tz::terminate();
	return 0;
}