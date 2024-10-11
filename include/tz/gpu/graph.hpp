#ifndef TOPAZ_GPU_GRAPH_HPP
#define TOPAZ_GPU_GRAPH_HPP
#include "tz/gpu/pass.hpp"

namespace tz::gpu
{
	/**
	 * @ingroup tz_gpu
	 * @defgroup tz_gpu_graph Graphs
	 * @brief Documentation for render graphs - describes the execution of @ref tz_gpu_pass.
	 */

	using graph_handle = tz::handle<pass_handle>;

	struct graph_info
	{
		std::span<const pass_handle> timeline = {};
		std::span<std::span<const pass_handle>> dependencies = {};
	};

	std::expected<graph_handle, tz::error_code> create_graph(graph_info graph);
	void execute(graph_handle);
	void destroy_graph(graph_handle);
}

#endif // TOPAZ_GPU_GRAPH_HPP