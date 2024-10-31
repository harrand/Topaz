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
		std::size_t quad_count = 0;
	};

	struct quad_data
	{
		tz::v4f pos_scale = {0.0f, 0.0f, 1.0f, 1.0f};
	};

	constexpr std::size_t initial_quad_capacity = 1024;

	std::vector<quad_renderer_data> renderers;
	tz::gpu::shader_handle main_pass_shader = tz::nullhand;

	std::expected<quad_renderer_handle, tz::error_code> create_quad_renderer(quad_renderer_info info)
	{
		if(main_pass_shader == tz::nullhand)
		{
			main_pass_shader = tz_must(tz::gpu::create_graphics_shader(ImportedShaderSource(quad, vertex), ImportedShaderSource(quad, fragment)));
		}

		std::size_t id = renderers.size();
		auto& ren = renderers.emplace_back();
		ren.info = info;

		std::array<quad_data, initial_quad_capacity> initial_quad_data;
		ren.data_buffer = tz_must(tz::gpu::create_buffer
		({
			.data = std::as_bytes(std::span<const quad_data>(initial_quad_data)),
			.name = "Quad Renderer Main Pass",
			.flags = tz::gpu::buffer_flag::dynamic_access
		}));

		tz::gpu::resource_handle colour_targets[] = 
		{
			tz::gpu::window_resource
		};
		tz::gpu::resource_handle resources[] =
		{
			ren.data_buffer
		};
		auto maybe_pass = tz::gpu::create_pass
		({
			.graphics = 
			{
				.colour_targets = colour_targets,
				.flags = tz::gpu::graphics_flag::no_depth_test
			},
			.shader = main_pass_shader,
			.resources = resources
		});
		if(maybe_pass.has_value())
		{
			ren.main_pass = maybe_pass.value();
		}
		else
		{
			return std::unexpected(maybe_pass.error());
		}

		ren.graph = tz_must(tz::gpu::graph_builder{}
			.set_flags(tz::gpu::graph_flag::present_after)
			.add_pass(ren.main_pass)
			.build());

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
		quad_data new_data;
		new_data.pos_scale = {0.0f, 0.0f, 0.25f, 0.25f};
		(void)info;

		auto& ren = renderers[renh.peek()];
		tz::gpu::pass_set_triangle_count(ren.main_pass, (ren.quad_count + 1) * 2);
		tz_must(tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const quad_data>(&new_data, 1)), sizeof(quad_data) * ren.quad_count));

		return static_cast<tz::hanval>(ren.quad_count++);
	}

	tz::gpu::graph_handle quad_renderer_graph(quad_renderer_handle renh)
	{
		return renderers[renh.peek()].graph;
	}
}