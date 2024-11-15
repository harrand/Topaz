#ifndef TOPAZ_GPU_GRAPH_HPP
#define TOPAZ_GPU_GRAPH_HPP
#include "tz/gpu/pass.hpp"
#include <vector>

namespace tz::gpu
{
	/**
	 * @ingroup tz_gpu
	 * @defgroup tz_gpu_graph Graphs
	 * @brief Documentation for render graphs - describes the execution of @ref tz_gpu_pass.
	 */

	/**
	 * @ingroup tz_gpu_graph
	 * @brief Specifies optional, extra functionality for a graph.
	 */
	enum graph_flag
	{
	};

	constexpr graph_flag operator|(graph_flag lhs, graph_flag rhs)
	{
		return static_cast<graph_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(graph_flag lhs, graph_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	using graph_handle = tz::handle<pass_handle>;

	/**
	 * @ingroup tz_gpu_graph
	 * @brief Create a new graph, which can be used for rendering a frame.
	 */
	graph_handle create_graph();
	void graph_add_pass(graph_handle graph, pass_handle pass, std::span<const pass_handle> dependencies = {});
	void graph_add_subgraph(graph_handle graph, graph_handle subgraph);
	/**
	 * @ingroup tz_gpu_graph
	 * @brief Execute the graph - invoking all passes contained a single time.
	 */
	void execute(graph_handle);
}

#endif // TOPAZ_GPU_GRAPH_HPP