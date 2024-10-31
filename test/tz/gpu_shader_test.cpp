#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/shader.hpp"

#include ImportedShaderHeader(triangle, vertex)
#include ImportedShaderHeader(triangle, fragment)
#include ImportedShaderHeader(noop, compute)

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::gpu::shader_handle graphics = tz_must(tz::gpu::create_graphics_shader(ImportedShaderSource(triangle, vertex), ImportedShaderSource(triangle, fragment)));
	tz::gpu::shader_handle compute = tz_must(tz::gpu::create_compute_shader(ImportedShaderSource(noop, compute)));

	// its optional to destroy shaders. if you dont - terminate() *will* delete all remaining shaders.
	tz::gpu::destroy_shader(compute);
	//tz::gpu::destroy_shader(graphics);

	tz::terminate();

	return 0;
}
