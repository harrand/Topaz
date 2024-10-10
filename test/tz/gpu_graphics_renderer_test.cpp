#include "tz/topaz.hpp"
#include "tz/os/window.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/pass.hpp"
#include ImportedTextHeader(empty_vertex, spv)
#include ImportedTextHeader(empty_fragment, spv)

int main()
{
	tz::initialise();
	tz::os::open_window({.name = "Graphics Render Test", .flags = tz::os::window_flags::invisible});

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
			.flags = tz::gpu::image_flags::render_target
		})),
	};

	tz::gpu::pass_handle pass = tz_must(tz::gpu::create_pass
	({
		.graphics =
		{
			.clear_colour = {1.0f, 1.0f, 1.0f},
			.colour_targets = colour_targets,
		},
		.shader = graphics,
	}));
	tz::gpu::destroy_pass(pass);

	tz::terminate();
	return 0;
}