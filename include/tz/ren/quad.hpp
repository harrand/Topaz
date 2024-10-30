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

	};
	std::expected<quad_handle, tz::error_code> quad_renderer_create_quad(quad_renderer_handle renh, quad_info info);

	tz::gpu::graph_handle quad_renderer_graph(quad_renderer_handle renh);
}

#endif // TOPAZ_REN_QUAD_HPP