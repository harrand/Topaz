#include "tz/dbgui/dbgui.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/matrix_transform.hpp"
#include <type_traits>

namespace tz
{
	template<typename T>
	concept has_dbgui_method = requires(T t)
	{
		{t.dbgui()} -> std::same_as<void>;
	};

	template<typename T>
	std::size_t transform_hierarchy<T>::size() const
	{
		return this->nodes.size();
	}

	template<typename T>
	void transform_hierarchy<T>::clear()
	{
		this->nodes.clear();
	}

	template<typename T>
	std::vector<unsigned int> transform_hierarchy<T>::get_root_node_ids() const
	{
		std::vector<unsigned int> ret;
		// for each element, check if any element has that as a child.
		// if nobody does, its a root node.
		for(std::size_t i = 0; i < this->size(); i++)
		{
			bool found_child = false;
			for(std::size_t j = 0; j < this->size(); j++)
			{
				if(i == j) continue;
				const auto& jnode = this->nodes[j];
				auto iter = std::find(jnode.children.begin(), jnode.children.end(), i);
				if(iter != jnode.children.end())
				{
					// i is a child of j
					// skip this one, its not a root node.
					found_child = true;
					break;
				}
			}
			if(!found_child)
			{
				ret.push_back(i);
			}
		}
		return ret;
	}

	template<typename T>
	unsigned int transform_hierarchy<T>::add_node(tz::trs local_transform, T data, std::optional<unsigned int> parent)
	{
		auto id = this->nodes.size();
		this->nodes.push_back({.data = data, .local_transform = local_transform, .parent = parent});
		if(parent.has_value())
		{
			this->nodes[parent.value()].children.push_back(id);
		}
		return id;
	}

	template<typename T>
	unsigned int transform_hierarchy<T>::add_hierarchy(const transform_hierarchy<T>& tree)
	{
		unsigned int offset = this->size();
		for(std::size_t i = 0; i < tree.size(); i++)
		{
			auto node = tree.get_node(i);
			if(node.parent.has_value())
			{
				node.parent.value() += offset;
			}
			this->add_node(node.local_transform, node.data, node.parent);
		}
		return offset;
	}

	template<typename T>
	unsigned int transform_hierarchy<T>::add_hierarchy_onto(const transform_hierarchy<T>& tree, unsigned int node_id)
	{
		unsigned int offset = this->size();
		for(std::size_t i = 0; i < tree.size(); i++)
		{
			auto node = tree.get_node(i);
			if(node.parent.has_value())
			{
				node.parent.value() += offset;
			}
			else
			{
				node.parent = node_id;
			}
			this->add_node(node.local_transform, node.data, node.parent);
		}
		return offset;
	}

	template<typename T>
	void expand_children(const transform_hierarchy<T>& src, transform_hierarchy<T>& hier, unsigned int src_id, unsigned int node_id)
	{
		const auto& src_node = src.get_node(src_id);
		const auto& node = hier.get_node(node_id);
		for(std::size_t i = 0; i < src_node.children.size(); i++)
		{
			// get the children of the source node. make a copy for the dest node and recursce on children.
			unsigned int src_child = src_node.children[i];
			const auto& src_child_node = src.get_node(src_child);
			unsigned int child = hier.add_node(src_child_node.local_transform, src_child_node.data, node_id);
			expand_children(src, hier, src_child, child);
		}
	}

	template<typename T>
	transform_hierarchy<T> transform_hierarchy<T>::export_node(unsigned int id) const
	{
		auto node = this->get_node(id);
		transform_hierarchy<T> ret;
		unsigned int new_root = ret.add_node(this->get_global_transform(id), node.data);
		expand_children<T>(*this, ret, id, new_root);
		return ret;
	}

	template<typename T>
	const transform_node<T>& transform_hierarchy<T>::get_node(unsigned int id) const
	{
		tz::assert(id < this->nodes.size(), "Invalid node id %u", id);
		return this->nodes[id];
	}

	template<typename T>
	tz::trs transform_hierarchy<T>::get_global_transform(unsigned int id) const
	{
		const auto& n = this->get_node(id);
		tz::trs global = n.local_transform;
		std::optional<unsigned int> p = n.parent;
		while(p.has_value())
		{
			const auto& pn = this->get_node(p.value());
			global = get_global_transform(p.value()).combined(global);
			p = pn.parent;
		}
		return global;
	}

	template<typename T>
	void transform_hierarchy<T>::iterate_children(unsigned int id, tz::action<unsigned int> auto callback) const
	{
		const auto& node = this->get_node(id);
		for(unsigned int child : node.children)
		{
			callback(child);
		}
	}

	template<typename T>
	void transform_hierarchy<T>::iterate_descendants(unsigned int id, tz::action<unsigned int> auto callback) const
	{
		const auto& node = this->get_node(id);
		for(unsigned int child : node.children)
		{
			callback(child);
			this->iterate_descendants(child, callback);
		}
	}

	template<typename T>
	void transform_hierarchy<T>::iterate_ancestors(unsigned int id, tz::action<unsigned int> auto callback) const
	{
		auto maybe_parent = this->get_node(id).parent;
		if(maybe_parent.has_value())
		{
			callback(maybe_parent.value());
			this->iterate_ancestors(maybe_parent.value(), callback);
		}
	}

	template<typename T>
	void transform_hierarchy<T>::iterate_nodes(tz::action<unsigned int> auto callback) const
	{
		auto root_node_ids = this->get_root_node_ids();
		for(unsigned int root_node : root_node_ids)
		{
			callback(root_node);
			this->iterate_descendants(root_node, callback);
		}
	}

	template<typename T>
	void transform_hierarchy<T>::dbgui()
	{
		for(unsigned int root : this->get_root_node_ids())
		{
			this->dbgui_node(root);
		}
	}

	template<typename T>
	void transform_hierarchy<T>::dbgui_node(unsigned int node_id)
	{
		const transform_node<T>& node = this->get_node(node_id);
		std::string node_name = "Node " + std::to_string(node_id);
		if(ImGui::TreeNode(node_name.c_str()))
		{
			if constexpr(has_dbgui_method<T>)
			{
				node.data.dbgui();
			}
			tz::mat4 local = node.local_transform.matrix();
			tz::mat4 global = this->get_global_transform(node_id).matrix();
			if(local != tz::mat4::identity())
			{
				ImGui::Text("Local Transform");
				tz::dbgui_model(local);
				ImGui::Text("Global Transform");
				tz::dbgui_model(global);
			}
			for(unsigned int child_idx : node.children)
			{
				this->dbgui_node(child_idx);
			}
			ImGui::TreePop();
		}
	}
}