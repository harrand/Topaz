#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/shader.hpp"

#include ImportedTextHeader(empty_vertex, spv)
#include ImportedTextHeader(empty_fragment, spv)
#include ImportedTextHeader(empty_compute, spv)

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::gpu::shader_handle graphics = tz_must(tz::gpu::create_graphics_shader(ImportedTextData(empty_vertex, spv), ImportedTextData(empty_fragment, spv)));
	tz::gpu::shader_handle compute = tz_must(tz::gpu::create_compute_shader(ImportedTextData(empty_compute, spv)));

	// its optional to destroy shaders. if you dont - terminate() *will* delete all remaining shaders.
	tz::gpu::destroy_shader(compute);
	//tz::gpu::destroy_shader(graphics);

	tz::terminate();

	return 0;
}
