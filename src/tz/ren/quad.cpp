#include "tz/ren/quad.hpp"
#include "tz/topaz.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/pass.hpp"
#include "tz/gpu/graph.hpp"
#include "tz/gpu/shader.hpp"

#include ImportedShaderHeader(quad, vertex)
#include ImportedShaderHeader(quad, fragment)

namespace tz::ren
{
	struct quad_renderer_data
	{
		quad_renderer_info info = {};
		tz::gpu::resource_handle data_buffer = tz::nullhand;
		tz::gpu::pass_handle main_pass = tz::nullhand;
		tz::gpu::graph_handle graph = tz::nullhand;
	};

	std::vector<quad_renderer_data> renderers;

	std::expected<quad_renderer_handle, tz::error_code> create_quad_renderer(quad_renderer_info info)
	{
		std::size_t id = renderers.size();
		auto& ren = renderers.emplace_back();
		ren.info = info;

		return static_cast<tz::hanval>(id);
	}

	tz::error_code destroy_quad_renderer(quad_renderer_handle renh)
	{
		auto& ren = renderers[renh.peek()];
		tz::gpu::destroy_pass(ren.main_pass);
		auto ret = tz::gpu::destroy_resource(ren.data_buffer);
		ren = {};
		return ret;
	}

	std::expected<quad_handle, tz::error_code> quad_renderer_create_quad(quad_renderer_handle renh, quad_info info)
	{
		(void)renh;
		(void)info;
		UNERR(tz::error_code::engine_bug, "create quad is NYI");
	}

	tz::gpu::graph_handle quad_renderer_graph(quad_renderer_handle renh)
	{
		return renderers[renh.peek()].graph;
	}
}