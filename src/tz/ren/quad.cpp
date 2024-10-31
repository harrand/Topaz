#include "tz/ren/quad.hpp"
#include "tz/core/matrix.hpp"
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
		tz::gpu::resource_handle camera_buffer = tz::nullhand;
		tz::gpu::pass_handle main_pass = tz::nullhand;
		tz::gpu::graph_handle graph = tz::nullhand;
		std::size_t quad_count = 0;
		std::size_t texture_count = 0;
	};

	struct quad_data
	{
		tz::v4f pos_scale = {0.0f, 0.0f, 1.0f, 1.0f};
		tz::v3f colour_tint = {1.0f, 1.0f, 1.0f};
		std::uint32_t texture_id = -1;
	};

	struct camera_data
	{
		tz::m4f projection = tz::m4f::iden();
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
			.name = "Quad Renderer Quad Buffer",
			.flags = tz::gpu::buffer_flag::dynamic_access
		}));

		camera_data initial_camera_data;
		ren.camera_buffer = tz_must(tz::gpu::create_buffer
		({
			.data = std::as_bytes(std::span<const camera_data>(&initial_camera_data, 1)),
			.name = "Quad Renderer Camera Buffer"
		}));

		tz::gpu::resource_handle colour_targets[] = 
		{
			tz::gpu::window_resource
		};
		tz::gpu::resource_handle resources[] =
		{
			ren.data_buffer,
			ren.camera_buffer
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
		if(ret != tz::error_code::success)
		{
			return ret;
		}
		ret = tz::gpu::destroy_resource(ren.camera_buffer);
		ren = {};
		return ret;
	}

	std::expected<quad_handle, tz::error_code> quad_renderer_create_quad(quad_renderer_handle renh, quad_info info)
	{
		quad_data new_data;
		new_data.pos_scale = {info.position[0], info.position[1], info.scale[0], info.scale[1]};
		new_data.colour_tint = info.colour;
		new_data.texture_id = info.texture_id;

		auto& ren = renderers[renh.peek()];

		if(info.texture_id != static_cast<unsigned int>(-1))
		{
			if(info.texture_id >= ren.texture_count)
			{
				UNERR(tz::error_code::invalid_value, "attempt to create quad with texture-id {}, but this is not a valid texture -- the quad renderer only has {} registered textures", info.texture_id, ren.texture_count);
			}
		}

		tz::gpu::pass_set_triangle_count(ren.main_pass, (ren.quad_count + 1) * 2);
		tz_must(tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const quad_data>(&new_data, 1)), sizeof(quad_data) * ren.quad_count));

		return static_cast<tz::hanval>(ren.quad_count++);
	}

	std::expected<std::uint32_t, tz::error_code> quad_renderer_add_texture(quad_renderer_handle renh, tz::gpu::resource_handle image)
	{
		auto& ren = renderers[renh.peek()];
		tz::error_code err = tz::gpu::pass_add_image_resource(ren.main_pass, image);
		if(err != tz::error_code::success)
		{
			return std::unexpected(err);
		}
		return ren.texture_count++;
	}

	tz::v2f get_quad_position(quad_renderer_handle renh, quad_handle quad)
	{
		const auto& ren = renderers[renh.peek()];
		auto quad_data_array = tz::gpu::resource_read(ren.data_buffer);
		tz::v4f pos_scale = *reinterpret_cast<const tz::v4f*>(quad_data_array.data() + (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, pos_scale));
		return {pos_scale[0], pos_scale[1]};
	}

	void set_quad_position(quad_renderer_handle renh, quad_handle quad, tz::v2f position)
	{
		auto& ren = renderers[renh.peek()];
		std::size_t offset = (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, pos_scale);

		tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const tz::v2f>(&position, 1)), offset);
	}

	tz::v2f get_quad_scale(quad_renderer_handle renh, quad_handle quad)
	{
		const auto& ren = renderers[renh.peek()];
		auto quad_data_array = tz::gpu::resource_read(ren.data_buffer);
		tz::v4f pos_scale = *reinterpret_cast<const tz::v4f*>(quad_data_array.data() + (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, pos_scale));
		return {pos_scale[2], pos_scale[3]};
	}

	void set_quad_scale(quad_renderer_handle renh, quad_handle quad, tz::v2f scale)
	{
		auto& ren = renderers[renh.peek()];
		std::size_t offset = (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, pos_scale) + sizeof(tz::v2f);

		tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const tz::v2f>(&scale, 1)), offset);
	}

	tz::v3f get_quad_colour(quad_renderer_handle renh, quad_handle quad)
	{
		const auto& ren = renderers[renh.peek()];
		auto quad_data_array = tz::gpu::resource_read(ren.data_buffer);
		return *reinterpret_cast<const tz::v3f*>(quad_data_array.data() + (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, colour_tint));
	}

	void set_quad_colour(quad_renderer_handle renh, quad_handle quad, tz::v3f colour)
	{
		auto& ren = renderers[renh.peek()];
		std::size_t offset = (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, colour_tint);

		tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const tz::v3f>(&colour, 1)), offset);
	}

	std::uint32_t get_quad_texture(quad_renderer_handle renh, quad_handle quad)
	{
		const auto& ren = renderers[renh.peek()];
		auto quad_data_array = tz::gpu::resource_read(ren.data_buffer);
		return *reinterpret_cast<const std::uint32_t*>(quad_data_array.data() + (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, texture_id));
	}

	void set_quad_texture(quad_renderer_handle renh, quad_handle quad, std::uint32_t texture_id)
	{
		auto& ren = renderers[renh.peek()];
		std::size_t offset = (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, texture_id);

		tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const std::uint32_t>(&texture_id, 1)), offset);
	}

	tz::gpu::graph_handle quad_renderer_graph(quad_renderer_handle renh)
	{
		return renderers[renh.peek()].graph;
	}

	void quad_renderer_update(quad_renderer_handle renh)
	{
		auto& ren = renderers[renh.peek()];
		(void)ren;
	}
}