#include "tz/dbgui/dbgui.hpp"
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
	const transform_node<T>& transform_hierarchy<T>::get_node(unsigned int id) const
	{
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
			ImGui::Text("Local Transform");
			tz::dbgui_model(local);
			ImGui::Text("Global Transform");
			tz::dbgui_model(global);
			for(unsigned int child_idx : node.children)
			{
				this->dbgui_node(child_idx);
			}
			ImGui::TreePop();
		}
	}
}