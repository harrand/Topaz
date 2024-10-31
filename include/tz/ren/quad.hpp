#ifndef TOPAZ_REN_QUAD_HPP
#define TOPAZ_REN_QUAD_HPP
#include "tz/core/handle.hpp"
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

	struct quad_renderer_info
	{

	};
	
	std::expected<quad_renderer_handle, tz::error_code> create_quad_renderer(quad_renderer_info info);
	tz::error_code destroy_quad_renderer(quad_renderer_handle renh);

	struct quad_info
	{
		tz::v2f position = tz::v2f::zero();
		float rotation = 0.0f;
		tz::v2f scale = tz::v2f::filled(1.0f);
	};
	std::expected<quad_handle, tz::error_code> quad_renderer_create_quad(quad_renderer_handle renh, quad_info info);

	tz::v2f get_quad_position(quad_renderer_handle renh, quad_handle quad);
	void set_quad_position(quad_renderer_handle renh, quad_handle quad, tz::v2f position);

	tz::v2f get_quad_scale(quad_renderer_handle renh, quad_handle quad);
	void set_quad_scale(quad_renderer_handle renh, quad_handle quad, tz::v2f scale);

	tz::gpu::graph_handle quad_renderer_graph(quad_renderer_handle renh);
}

#endif // TOPAZ_REN_QUAD_HPP