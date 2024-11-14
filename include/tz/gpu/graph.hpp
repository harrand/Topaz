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
	 * @brief Specifies creation flags for a new graph.
	 *
	 * See @ref tz::gpu::create_graph for usage.
	 * @note You may prefer to use the helper struct @ref graph_builder to generate a graph instead.
	 */
	struct graph_info
	{
		/// Contains all the passes that will be executed during a single frame - in chronological order.
		std::span<const pass_handle> timeline = {};
		/// List of dependencies for each pass in the timeline. The n'th index of dependencies corresponds to the dependencies for the pass at the n'th index of the timeline.
		std::span<std::span<const pass_handle>> dependencies = {};
		/// Specifies extra optional behaviour for the graph.
		graph_flag flags = static_cast<graph_flag>(0);
		/// Optional. Executed when the graph is executed.
		void(*on_execute)(graph_handle graph) = nullptr;
	};

	/**
	 * @ingroup tz_gpu_graph
	 * @brief Helper struct for creating a new graph. Follows the builder pattern.
	 *
	 * To populate the fields in @ref graph_info manually will require you to lay out the passes in memory in a particular way, so to avoid allocations. However, if you're happy to accept an allocation or two, you can use this helper builder to construct the graph instead.
	 *
	 */
	struct graph_builder
	{
		std::vector<pass_handle> passes;
		std::vector<std::vector<pass_handle>> dependencies{};
		graph_flag flags = static_cast<graph_flag>(0);

		/// Add a new pass to the end of the timeline.
		graph_builder& add_pass(pass_handle pass)
		{
			this->passes.push_back(pass);
			this->dependencies.push_back({});
			return *this;
		}

		/// Add a new dependency to an existing pass in the timeline. Make sure the pass has already been added via @ref add_pass.
		graph_builder& add_dependency(pass_handle pass, pass_handle dependency)
		{
			auto iter = std::find(this->passes.begin(), this->passes.end(), pass);
			if(iter != this->passes.end())
			{
				auto id = std::distance(this->passes.begin(), iter);
				dependencies[id].push_back(dependency);
			}
			return *this;
		}

		graph_builder& set_flags(graph_flag flags)
		{
			this->flags = flags;
			return *this;
		}
		/// Attempt to create the graph based on all previous calls and return the result. This will call @ref create_graph for you.
		inline std::expected<graph_handle, tz::error_code> build();
	};

	/**
	 * @ingroup tz_gpu_graph
	 * @brief Create a new graph, which can be used for rendering a frame.
	 * @return @ref tz::error_code::invalid_value If you define a number of sets of dependencies that is greater than the timeline size. That is - the number of sets of dependencies should be less than or equal to the total number of passes in the graph.
	 */
	std::expected<graph_handle, tz::error_code> create_graph(graph_info graph);
	/**
	 * @ingroup tz_gpu_graph
	 * @brief Execute the graph - invoking all passes contained a single time.
	 */
	void execute(graph_handle);


	std::expected<graph_handle, tz::error_code> graph_builder::build()
	{
		graph_info i
		{
			.timeline = this->passes,
			.flags = this->flags
		};
		std::vector<std::span<const pass_handle>> dependency_pointers;
		for(const auto& dep_list : dependencies)
		{
			dependency_pointers.push_back(dep_list);
		}
		i.dependencies = dependency_pointers;
		return tz::gpu::create_graph(i);
	}
}

#endif // TOPAZ_GPU_GRAPH_HPP