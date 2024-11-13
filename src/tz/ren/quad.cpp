#include "tz/ren/quad.hpp"
#include "tz/core/matrix.hpp"
#include "tz/core/trs.hpp"
#include "tz/topaz.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/pass.hpp"
#include "tz/gpu/graph.hpp"
#include "tz/gpu/shader.hpp"
#include "tz/os/window.hpp"

#include ImportedShaderHeader(quad, vertex)
#include ImportedShaderHeader(quad, fragment)

namespace tz::ren
{
	struct quad_internal_data
	{
		tz::trs transform;
		bool garbage = false;
	};

	struct quad_renderer_data
	{
		quad_renderer_info info = {};
		tz::gpu::resource_handle data_buffer = tz::nullhand;
		tz::gpu::resource_handle camera_buffer = tz::nullhand;
		tz::gpu::resource_handle settings_buffer = tz::nullhand;
		tz::gpu::pass_handle main_pass = tz::nullhand;
		tz::gpu::graph_handle graph = tz::nullhand;
		std::vector<quad_internal_data> internals = {};
		std::size_t quad_count = 0;
		std::size_t texture_count = 0;
		unsigned int window_width_cache;
		unsigned int window_height_cache;
		std::vector<std::size_t> free_list = {};
	};

	struct quad_data
	{
		tz::m4f model = tz::m4f::iden();
		tz::v3f colour = {1.0f, 1.0f, 1.0f};
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

		auto settings_val = static_cast<std::uint32_t>(info.flags);
		ren.settings_buffer = tz_must(tz::gpu::create_buffer
		({
			.data = std::as_bytes(std::span<const std::uint32_t>(&settings_val, 1)),
			.name = "Quad Renderer Settings Buffer"
		}));

		tz::gpu::resource_handle colour_targets[] = 
		{
			info.colour_target
		};
		tz::gpu::resource_handle resources[] =
		{
			ren.data_buffer,
			ren.camera_buffer,
			ren.settings_buffer
		};
		auto maybe_pass = tz::gpu::create_pass
		({
			.graphics = 
			{
				.clear_colour = info.clear_colour,
				.colour_targets = colour_targets,
				.culling = (info.flags & quad_renderer_flag::allow_negative_scale) ? tz::gpu::cull::none : tz::gpu::cull::back,
				.flags = tz::gpu::graphics_flag::no_depth_test
			},
			.shader = main_pass_shader,
			.resources = resources,
		});
		if(maybe_pass.has_value())
		{
			ren.main_pass = maybe_pass.value();
		}
		else
		{
			return std::unexpected(maybe_pass.error());
		}

		auto builder = tz::gpu::graph_builder{}
		.add_pass(ren.main_pass);
		if(info.flags & quad_renderer_flag::graph_present_after)
		{
			builder.add_pass(tz::gpu::present_pass);
		}
		ren.graph = tz_must(builder.build());

		ren.window_width_cache = 0;
		ren.window_height_cache = 0;

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
		auto& ren = renderers[renh.peek()];
		std::size_t id = ren.quad_count;
		bool recycled = false;
		if(ren.free_list.size())
		{
			recycled = true;
			id = ren.free_list.back();
			ren.free_list.pop_back();
		}
		else
		{
			ren.internals.push_back({});
			ren.quad_count++;
		}
		quad_internal_data& internal = ren.internals[id];
		internal = {};
		internal.transform =
		{
			.translate = {info.position[0], info.position[1], 0.0f},
			.rotate = tz::quat::from_axis_angle({0.0f, 0.0f, 1.0f}, info.rotation),
			.scale = {info.scale[0], info.scale[1], 1.0f}
		};

		quad_data new_data;
		new_data.model = internal.transform.matrix();
		new_data.colour = info.colour;
		new_data.texture_id = info.texture_id;

		if(info.texture_id != static_cast<unsigned int>(-1))
		{
			if(info.texture_id >= ren.texture_count)
			{
				UNERR(tz::error_code::invalid_value, "attempt to create quad with texture-id {}, but this is not a valid texture -- the quad renderer only has {} registered textures", info.texture_id, ren.texture_count);
			}
		}

		if(!recycled)
		{
			tz::gpu::pass_set_triangle_count(ren.main_pass, ren.quad_count * 2);
		}
		tz_must(tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const quad_data>(&new_data, 1)), sizeof(quad_data) * id));

		return static_cast<tz::hanval>(id);
	}

	tz::error_code quad_renderer_destroy_quad(quad_renderer_handle renh, quad_handle quad)
	{
		// quad destruction works as follows:
		// set the internal quad data to indicate that the quad is garbage.
		// set the scale to zero (the draw is unchanged but the data makes it invisible)
		// add to the free list, the next time a quad is created this handle can be re-used.
		auto& ren = renderers[renh.peek()];
		auto& q = ren.internals[quad.peek()];
		if(q.garbage)
		{
			RETERR(tz::error_code::precondition_failure, "attempt to destroy quad {} but it was already destroyed. double delete.", quad.peek());
		}
		q = {};
		q.garbage = true;
		set_quad_scale(renh, quad, tz::v2f::zero());
		ren.free_list.push_back(quad.peek());
		return tz::error_code::success;
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
		tz::v3f pos = ren.internals[quad.peek()].transform.translate;
		return {pos[0], pos[1]};
	}

	void set_quad_position(quad_renderer_handle renh, quad_handle quad, tz::v2f position)
	{
		auto& ren = renderers[renh.peek()];
		auto& internal = ren.internals[quad.peek()];
		internal.transform.translate = {position[0], position[1]};

		tz::m4f model = internal.transform.matrix();
		tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const tz::m4f>(&model, 1)), sizeof(quad_data) * quad.peek() + offsetof(quad_data, model));
	}

	tz::v2f get_quad_scale(quad_renderer_handle renh, quad_handle quad)
	{
		const auto& ren = renderers[renh.peek()];
		tz::v3f scale = ren.internals[quad.peek()].transform.scale;
		return {scale[0], scale[1]};
	}

	void set_quad_scale(quad_renderer_handle renh, quad_handle quad, tz::v2f scale)
	{
		auto& ren = renderers[renh.peek()];
		auto& internal = ren.internals[quad.peek()];
		internal.transform.scale = {scale[0], scale[1]};

		tz::m4f model = internal.transform.matrix();
		tz::gpu::resource_write(ren.data_buffer, std::as_bytes(std::span<const tz::m4f>(&model, 1)), sizeof(quad_data) * quad.peek() + offsetof(quad_data, model));
	}

	tz::v3f get_quad_colour(quad_renderer_handle renh, quad_handle quad)
	{
		const auto& ren = renderers[renh.peek()];
		auto quad_data_array = tz::gpu::resource_read(ren.data_buffer);
		return *reinterpret_cast<const tz::v3f*>(quad_data_array.data() + (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, colour));
	}

	void set_quad_colour(quad_renderer_handle renh, quad_handle quad, tz::v3f colour)
	{
		auto& ren = renderers[renh.peek()];
		std::size_t offset = (sizeof(quad_data) * quad.peek()) + offsetof(quad_data, colour);

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

		auto w = tz::gpu::image_get_width(ren.info.colour_target);
		auto h = tz::gpu::image_get_height(ren.info.colour_target);
		if((w != 0 && h != 0) && (ren.window_width_cache != w || ren.window_height_cache != h))
		{
			// window has resized
			ren.window_width_cache = w;
			ren.window_height_cache = h;
			// todo: regenerate projection matrix.
			auto aspect_ratio = static_cast<float>(w) / h;
			tz::m4f projection = tz::matrix_ortho(-1 * aspect_ratio, 1 * aspect_ratio, 1, -1, -1, 1);
			tz::gpu::resource_write(ren.camera_buffer, std::as_bytes(std::span<const tz::m4f>(&projection, 1)), offsetof(camera_data, projection));
		}
	}
}