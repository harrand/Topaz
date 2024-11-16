#include "tz/topaz.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/pass.hpp"
#include "tz/gpu/graph.hpp"
#include "tz/gpu/shader.hpp"

#include "tz/imgui.hpp"

#include ImportedShaderHeader(triangle, vertex)
#include ImportedShaderHeader(triangle, fragment)

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();
	tz::os::open_window({.title = "tz_triangle_demo"});

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
			.depth_target = tz::gpu::window_resource,
			.triangle_count = 1
		},
		.shader = graphics,
		.name = "Render Triangle"
	}));

	tz::gpu::graph_handle graph = tz::gpu::create_graph("Main Graph");
	tz::gpu::graph_add_pass(graph, pass);
	//tz::gpu::graph_add_subgraph(graph, tz::imgui_render_graph());
	tz::gpu::graph_add_pass(graph, tz::gpu::present_pass);

	while(tz::os::window_is_open())
	{
		tz::os::window_update();
		tz::gpu::execute(graph);
	}

	tz::terminate();
	return 0;
}