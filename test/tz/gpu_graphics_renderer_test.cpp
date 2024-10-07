#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/pass.hpp"
#include ImportedTextHeader(empty_vertex, spv)
#include ImportedTextHeader(empty_fragment, spv)

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::gpu::shader_handle graphics = tz_must(tz::gpu::create_graphics_shader(ImportedTextData(empty_vertex, spv), ImportedTextData(empty_fragment, spv)));

	tz::gpu::pass_handle pass = tz_must(tz::gpu::create_pass
	({
		.graphics = {.clear_colour = {1.0f, 1.0f, 1.0f}},
		.shader = graphics
	}));
	tz::gpu::destroy_pass(pass);

	tz::terminate();
	return 0;
}