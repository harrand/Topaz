#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/pass.hpp"
#include "tz/gpu/graph.hpp"
#include ImportedShaderHeader(noop, compute)

#include "tz/main.hpp"
int tz_main()
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

	tz::gpu::graph_handle graph = tz::gpu::create_graph();
	tz::gpu::graph_add_pass(graph, pass);

	for(std::size_t i = 0; i < 64; i++)
	{
		tz::gpu::execute(graph);
	}

	tz::gpu::destroy_pass(pass);

	tz::terminate();
	return 0;
}