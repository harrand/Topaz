#ifndef TZ_CORE_DATA_TRANSFORM_HIERARCHY_HPP
#define TZ_CORE_DATA_TRANSFORM_HIERARCHY_HPP
#include "tz/core/data/trs.hpp"
#include "tz/core/data/handle.hpp"
#include <optional>
#include <cstddef>

namespace tz
{
	template<typename T>
	struct transform_node
	{
		mutable T data;
		mutable tz::trs local_transform = {};
		std::optional<unsigned int> parent = std::nullopt;
		std::vector<unsigned int> children = {};
	};

	template<typename T = void*>
	class transform_hierarchy
	{
	public:
		transform_hierarchy() = default;
		std::size_t size() const;
		bool empty() const{return this->size() == 0;}
		std::vector<unsigned int> get_root_node_ids() const;

		// returns id of the new node.
		unsigned int add_node(tz::trs local_transform = {}, T data = {}, std::optional<unsigned int> parent = std::nullopt);
		// returns offset to be applied to the previous hierarchy's set of nodes to get their corresponding node ids
		// within this hierarchy
		unsigned int add_hierarchy(const transform_hierarchy<T>& tree);
		unsigned int add_hierarchy_onto(const transform_hierarchy<T>& tree, unsigned int node_id);
		const transform_node<T>& get_node(unsigned int id) const;
		tz::trs get_global_transform(unsigned int id) const;

		void dbgui();
	private:
		void dbgui_node(unsigned int node_id);
		std::vector<transform_node<T>> nodes = {};
	};
}

#include "tz/core/data/transform_hierarchy.inl"
#endif // TZ_CORE_DATA_TRANSFORM_HIERARCHY_HPP