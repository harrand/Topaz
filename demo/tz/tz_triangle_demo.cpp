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
	tz::os::open_window({.title = "tz_triangle_demo"});

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::gpu::shader_handle graphics = tz_must(tz::gpu::create_graphics_shader(ImportedShaderSource(triangle, vertex), ImportedShaderSource(triangle, fragment)));

	tz::gpu::resource_handle colour_targets[] =
	{
		tz::gpu::window_resource,
	};

	tz::v4f clear_colour = {1.0f, 1.0f, 0.5f, 1.0f};
	std::array<std::uint32_t, 4> imgdata =
	{
		0xFFFFFFFF,
		0xFFFFFF00,
		0xFFFFFF00,
		0xFFFFFFFF,
	};
	
	tz::gpu::resource_handle resources[] =
	{
		tz_must(tz::gpu::create_buffer
		({
			.data = std::as_bytes(std::span<const tz::v4f>(&clear_colour, 1))
		})),
		tz_must(tz::gpu::create_image
		({
			.width = 2,
			.height = 2,
			.data = std::as_bytes(std::span<const std::uint32_t>(imgdata)),
		}))
	};

	tz::gpu::pass_handle pass = tz_must(tz::gpu::create_pass
	({
		.graphics =
		{
			.clear_colour = {0.0f, 0.0f, 0.0f, 1.0f},
			.colour_targets = colour_targets,
			.depth_target = tz::gpu::window_resource
		},
		.shader = graphics,
		.resources = resources
	}));

	tz::gpu::graph_handle graph = tz_must(tz::gpu::graph_builder{}
		.set_flags(tz::gpu::graph_flag::present_after)
		.add_pass(pass)
		.build());

	std::size_t counter = 0;
	while(tz::os::window_is_open())
	{
		tz::os::window_update();
		tz::gpu::execute(graph);
		counter++;

		clear_colour[0] = std::sin(counter * 0.01f);
		tz::gpu::resource_write(resources[0], std::as_bytes(std::span<const tz::v4f>(&clear_colour, 1)));
	}

	tz::gpu::destroy_pass(pass);
	tz::terminate();
	return 0;
}