#ifndef TOPAZ_REN_QUAD_HPP
#define TOPAZ_REN_QUAD_HPP
#include "tz/core/handle.hpp"
#include "tz/core/vector.hpp"
#include "tz/gpu/graph.hpp"

namespace tz::ren
{
	namespace detail
	{
		struct quad_t{};
		struct quadren_t{};
	}
	using quad_handle = tz::handle<detail::quad_t>;
	using quad_renderer_handle = tz::handle<detail::quadren_t>;

	enum quad_renderer_flag
	{
		alpha_clipping = 0b0001,
	};

	constexpr quad_renderer_flag operator|(quad_renderer_flag lhs, quad_renderer_flag rhs)
	{
		return static_cast<quad_renderer_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(quad_renderer_flag lhs, quad_renderer_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	struct quad_renderer_info
	{
		tz::v4f clear_colour = {0.0f, 0.0f, 0.0f, 1.0f};
		quad_renderer_flag flags = static_cast<quad_renderer_flag>(0);
		tz::gpu::resource_handle colour_target = tz::gpu::window_resource;
	};
	
	std::expected<quad_renderer_handle, tz::error_code> create_quad_renderer(quad_renderer_info info);
	tz::error_code destroy_quad_renderer(quad_renderer_handle renh);

	struct quad_info
	{
		tz::v2f position = tz::v2f::zero();
		float rotation = 0.0f;
		tz::v2f scale = tz::v2f::filled(1.0f);
		std::uint32_t texture_id = -1;
		tz::v3f colour = tz::v3f::filled(1.0f);
	};
	std::expected<quad_handle, tz::error_code> quad_renderer_create_quad(quad_renderer_handle renh, quad_info info);

	std::expected<std::uint32_t, tz::error_code> quad_renderer_add_texture(quad_renderer_handle renh, tz::gpu::resource_handle image);

	tz::v2f get_quad_position(quad_renderer_handle renh, quad_handle quad);
	void set_quad_position(quad_renderer_handle renh, quad_handle quad, tz::v2f position);

	tz::v2f get_quad_scale(quad_renderer_handle renh, quad_handle quad);
	void set_quad_scale(quad_renderer_handle renh, quad_handle quad, tz::v2f scale);

	tz::v3f get_quad_colour(quad_renderer_handle renh, quad_handle quad);
	void set_quad_colour(quad_renderer_handle renh, quad_handle quad, tz::v3f colour);

	std::uint32_t get_quad_texture(quad_renderer_handle renh, quad_handle quad);
	void set_quad_texture(quad_renderer_handle renh, quad_handle quad, std::uint32_t texture_id);

	tz::gpu::graph_handle quad_renderer_graph(quad_renderer_handle renh);
	void quad_renderer_update(quad_renderer_handle renh);
}

#endif // TOPAZ_REN_QUAD_HPP