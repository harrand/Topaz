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

	using graph_handle = tz::handle<pass_handle>;

	struct graph_info
	{
		std::span<const pass_handle> timeline = {};
		std::span<std::span<const pass_handle>> dependencies = {};
	};

	struct graph_builder
	{
		std::vector<pass_handle> passes;
		std::vector<std::vector<pass_handle>> dependencies{};

		graph_builder& add_pass(pass_handle pass)
		{
			this->passes.push_back(pass);
			this->dependencies.push_back({});
			return *this;
		}

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
		inline std::expected<graph_handle, tz::error_code> build();
	};

	std::expected<graph_handle, tz::error_code> create_graph(graph_info graph);
	void execute(graph_handle);
	void destroy_graph(graph_handle);


	std::expected<graph_handle, tz::error_code> graph_builder::build()
	{
		graph_info i
		{
			.timeline = this->passes	
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