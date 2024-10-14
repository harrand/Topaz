#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/pass.hpp"
#include ImportedShaderHeader(noop, compute)

int main()
{
	tz::initialise();

	tz::gpu::hardware gpu = tz::gpu::find_best_hardware();
	tz_must(tz::gpu::use_hardware(gpu));

	tz::gpu::shader_handle shad = tz_must(tz::gpu::create_compute_shader(ImportedShaderSource(noop, compute)));

	tz::gpu::pass_handle pass = tz_must(tz::gpu::create_pass
	({
		.compute = {.kernel = {64u, 64u, 64u}},
		.shader = shad
	}));
	tz::gpu::destroy_pass(pass);

	tz::terminate();
	return 0;
}