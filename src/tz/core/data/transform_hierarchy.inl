#include "tz/dbgui/dbgui.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/core/job/job.hpp"
#include <set>
#include <type_traits>
#include <sstream>

namespace tz
{
	template<typename T>
	concept has_dbgui_method = requires(T t)
	{
		{t.dbgui()} -> std::same_as<void>;
	};

	template<typename T>
	concept is_printable = requires(T t)
	{
		{std::ostringstream{} << t};
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
		this->node_free_list.clear();
		this->value_node_map.clear();
	}

	template<typename T>
	std::vector<unsigned int> transform_hierarchy<T>::get_root_node_ids() const
	{
		TZ_PROFZONE("transform_hierarchy - get root node ids", 0xFF0000AA);
		std::vector<unsigned int> ret;
		// for each element, check if any element has that as a child.
		// if nobody does, its a root node.

		// linear approach: iterate through each node and collate a list of all children ever. iterate through again and filter out any that appear in the list of children.
		std::set<unsigned int> all_children;
		for(std::size_t i = 0; i < this->size(); i++)
		{
			const auto& node = this->nodes[i];
			for(auto child : node.children)
			{
				all_children.insert(child);
			}
		}

		for(std::size_t i = 0; i < this->size(); i++)
		{
			if(!all_children.contains(i))
			{
				ret.push_back(i);
			}
		}

		// quadratic approach is: for each element a, for each element b, is a any of bs children? if never then a is root node
		//for(std::size_t i = 0; i < this->size(); i++)
		//{
		//	bool found_child = false;
		//	for(std::size_t j = 0; j < this->size(); j++)
		//	{
		//		if(i == j) continue;
		//		TZ_PROFZONE("get root node ids - quadratic iterate", 0xFF0000AA);
		//		const auto& jnode = this->nodes[j];
		//		auto iter = std::find(jnode.children.begin(), jnode.children.end(), i);
		//		if(iter != jnode.children.end())
		//		{
		//			// i is a child of j
		//			// skip this one, its not a root node.
		//			found_child = true;
		//			break;
		//		}
		//	}
		//	if(!found_child)
		//	{
		//		ret.push_back(i);
		//	}
		//}
		return ret;
	}

	template<typename T>
	std::optional<unsigned int> transform_hierarchy<T>::find_node(const T& value) const
	{
		TZ_PROFZONE("transform_hierarchy - find node", 0xFF0000AA);
		auto iter = this->value_node_map.find(value);
		if(iter == this->value_node_map.end())
		{
			return std::nullopt;
		}
		return iter->second;
		/*
		auto iter = std::find_if(this->nodes.begin(), this->nodes.end(),
		[value](const auto& node)
		{
			return node.data == value;
		});
		if(iter == this->nodes.end())
		{
			return std::nullopt;
		}
		return std::distance(this->nodes.begin(), iter);
		*/
	}

	template<typename T>
	std::optional<unsigned int> transform_hierarchy<T>::find_node_if(tz::function<bool, const T&> auto predicate) const
	{
		TZ_PROFZONE("transform_hierarchy - find node if", 0xFF0000AA);
		auto iter = std::find_if(this->nodes.begin(), this->nodes.end(),
		[predicate](const auto& node)
		{
			return predicate(node.data);
		});
		if(iter == this->nodes.end())
		{
			return std::nullopt;
		}
		return std::distance(this->nodes.begin(), iter);
	}

	template<typename T>
	unsigned int transform_hierarchy<T>::add_node(tz::trs local_transform, T data, std::optional<unsigned int> parent)
	{
		TZ_PROFZONE("transform_hierarchy - add node", 0xFF0000AA);
		std::size_t id;
		if(this->node_free_list.size())
		{
			id = this->node_free_list.front();
			tz::report("Use recycled node id %zu", id);
			this->node_free_list.erase(this->node_free_list.begin());
			this->nodes[id] = {.data = data, .local_transform = local_transform, .parent = parent};
		}
		else
		{
			id = this->nodes.size();
			this->nodes.push_back({.data = data, .local_transform = local_transform, .parent = parent});
			this->node_local_transform_hashes.push_back(std::numeric_limits<std::size_t>::max());
			this->node_global_transform_cache.push_back({});
		}
		if(parent.has_value())
		{
			this->nodes[parent.value()].children.push_back(id);
		}
		tz::assert(this->value_node_map.find(data) == this->value_node_map.end(), "Node values must be unique. Detected there was already an existing node with the given value.");
		this->value_node_map[data] = id;
		return id;
	}

	template<typename T>
	void transform_hierarchy<T>::remove_node(unsigned int node_id, remove_strategy strategy)
	{
		TZ_PROFZONE("transform_hierarchy - remove node", 0xFF0000AA);
		tz::assert(std::find(this->node_free_list.begin(), this->node_free_list.end(), node_id) == this->node_free_list.end(), "Double remove detected on node %u", node_id);
		tz::report("Remove node %u", node_id);
		tz::assert(node_id < this->nodes.size(), "Invalid node_id %zu (node count: %zu)", node_id, this->nodes.size());
		// what if the node had a parent?
		const auto& node = this->get_node(node_id);
		if(node.parent.has_value())
		{
			auto& parent = this->nodes[node.parent.value()];
			parent.children.erase(std::remove(parent.children.begin(), parent.children.end(), node_id), parent.children.end());
		}
		this->value_node_map.erase(node.data);
		switch(strategy)
		{
			case remove_strategy::patch_children_to_parent:
				if(node.parent.has_value())
				{
					for(unsigned int child_node_idx : node.children)
					{
						auto& parent = this->nodes[node.parent.value()];
						this->nodes[child_node_idx].parent = node.parent.value();
						parent.children.push_back(child_node_idx);
					}
				}
				for(unsigned int child_node_idx : node.children)
				{
					this->nodes[child_node_idx].parent = std::nullopt;
				}
			break;
			case remove_strategy::remove_children:
			{
				// node.children will change each iteration, so work on a copy.
				while(node.children.size())
				{
					this->remove_node(node.children.front(), strategy);
				}
			}
			break;
			case remove_strategy::detach_children:
				for(unsigned int child_node_idx : node.children)
				{
					this->nodes[child_node_idx].parent = std::nullopt;
				}
			break;
			case remove_strategy::impl_do_nothing:
			break;
		}
		this->nodes[node_id] = {};
		this->node_free_list.push_back(node_id);
		this->clear_cache_for(node_id);
		//this->nodes.erase(this->nodes.begin() + node_id);
	}

	template<typename T>
	std::optional<unsigned int> transform_hierarchy<T>::node_get_parent(unsigned int node_id) const
	{
		return this->get_node(node_id).parent;
	}

	template<typename T>
	void transform_hierarchy<T>::node_set_parent(unsigned int node_id, unsigned int new_parent_node_id)
	{
		this->get_node(node_id).parent = new_parent_node_id;
		this->clear_cache_for(node_id);
	}

	template<typename T>
	void transform_hierarchy<T>::node_clear_parent(unsigned int node_id)
	{
		this->get_node(node_id).parent = std::nullopt;
		this->clear_cache_for(node_id);
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
		TZ_PROFZONE("transform_hierarchy - expand children", 0xFF0000AA);
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
		// dont instrument this. this gets called a **ton** even on tiny scenes.
		//TZ_PROFZONE("transform_hierarchy - get node", 0xFF0000AA);
		tz::assert(id < this->nodes.size(), "Invalid node id %u", id);
		return this->nodes[id];
	}

	template<typename T>
	transform_node<T>& transform_hierarchy<T>::get_node(unsigned int id)
	{
		// dont instrument this. this gets called a **ton** even on tiny scenes.
		//TZ_PROFZONE("transform_hierarchy - get node", 0xFF0000AA);
		tz::assert(id < this->nodes.size(), "Invalid node id %u", id);
		return this->nodes[id];
	}

	template<typename T>
	tz::trs transform_hierarchy<T>::get_global_transform(unsigned int id) const
	{
		TZ_PROFZONE("transform_hierarchy - get global transform", 0xFF0000AA);
		if(!this->node_cache_miss(id))
		{
			// cache is correct, just return that.
			return this->node_global_transform_cache[id];
		}
		//tz::report("transform hierarchy cache miss at %zu", id);
		const auto& n = this->get_node(id);
		// cache is incorrect. we should also dirty all children as our local has changed.
		for(unsigned int child : n.children)
		{
			clear_cache_for(child);
		}	
		tz::trs global = n.local_transform;
		std::optional<unsigned int> p = n.parent;
		if(p.has_value())
		{
			global.combine(get_global_transform(p.value()));
		}
		this->cache_write(id, global);
		return global;
	}

	template<typename T>
	void transform_hierarchy<T>::iterate_children(unsigned int id, tz::action<unsigned int> auto callback) const
	{
		TZ_PROFZONE("transform_hierarchy - iterate children", 0xFF0000AA);
		const auto& node = this->get_node(id);
		for(unsigned int child : node.children)
		{
			callback(child);
		}
	}

	template<typename T>
	void transform_hierarchy<T>::iterate_descendants(unsigned int id, tz::action<unsigned int> auto callback, bool callback_thread_safe) const
	{
		TZ_PROFZONE("transform_hierarchy - iterate descendants", 0xFF0000AA);
		const auto& node = this->get_node(id);

		if(callback_thread_safe && node.children.size() >= tz::job_system().worker_count())
		{
			// if we're allowed to spawn jobs to help, let's do it.
			const std::size_t job_count = tz::job_system().worker_count();
			std::size_t children_per_job = node.children.size() / job_count;
			std::size_t remainder_children = node.children.size() % job_count;

			std::vector<tz::job_handle> jobs;
			jobs.resize(job_count);
			for(std::size_t i = 0; i < job_count; i++)
			{
				jobs[i] = tz::job_system().execute([&node, &callback, this, offset = i * children_per_job, child_count = children_per_job]()
				{
					for(std::size_t j = 0; j < child_count; j++)
					{
						unsigned int child = node.children[j + offset];
						callback(child); this->iterate_descendants(child, callback, false);
					}
				});
			}
			for(std::size_t i = node.children.size() - remainder_children; i < node.children.size(); i++)
			{
				unsigned int child = node.children[i];
				callback(child);
				this->iterate_descendants(child, callback, false);
			}
			for(tz::job_handle jh : jobs)
			{
				tz::job_system().block(jh);
			}
		}
		else
		{
			for(unsigned int child : node.children)
			{
				callback(child);
				this->iterate_descendants(child, callback, callback_thread_safe);
			}
		}
	}

	template<typename T>
	void transform_hierarchy<T>::iterate_ancestors(unsigned int id, tz::action<unsigned int> auto callback) const
	{
		TZ_PROFZONE("transform_hierarchy - iterate ancestors", 0xFF0000AA);
		auto maybe_parent = this->get_node(id).parent;
		if(maybe_parent.has_value())
		{
			callback(maybe_parent.value());
			this->iterate_ancestors(maybe_parent.value(), callback);
		}
	}

	template<typename T>
	void transform_hierarchy<T>::iterate_nodes(tz::action<unsigned int> auto callback, bool callback_thread_safe) const
	{
		TZ_PROFZONE("transform_hierarchy - iterate nodes", 0xFF0000AA);
		auto root_node_ids = this->get_root_node_ids();
		for(unsigned int root_node : root_node_ids)
		{
			{
				TZ_PROFZONE("iterate nodes - invoke callback", 0xFF0000AA);
				callback(root_node);
			}
			TZ_PROFZONE("iterate nodes - iterate descendants", 0xFF0000AA);
			this->iterate_descendants(root_node, callback, callback_thread_safe);
		}
	}

	template<typename T>
	bool transform_hierarchy<T>::node_cache_miss(unsigned int node_id) const
	{
		std::size_t hash = std::hash<tz::trs>{}(this->get_node(node_id).local_transform);
		tz::assert(node_id < this->node_local_transform_hashes.size());
		if(hash == this->node_local_transform_hashes[node_id])
		{
			return false;
		}
		this->node_local_transform_hashes[node_id] = hash;
		return true;
	}

	template<typename T>
	void transform_hierarchy<T>::cache_write(unsigned int node_id, tz::trs global) const
	{
		this->node_global_transform_cache[node_id] = global;
	}

	template<typename T>
	void transform_hierarchy<T>::clear_cache_for(unsigned int node_id) const
	{
		this->node_local_transform_hashes[node_id] = std::numeric_limits<std::size_t>::max();
	}

	template<typename T>
	void transform_hierarchy<T>::clear_cache() const
	{
		for(auto& hash : this->node_local_transform_hashes)
		{
			hash = std::numeric_limits<std::size_t>::max();
		}
	}

	template<typename T>
	void transform_hierarchy<T>::dbgui(bool display_gizmo)
	{
		for(unsigned int root : this->get_root_node_ids())
		{
			if(!this->dbgui_node(root, display_gizmo))
			{
				break;
			}
		}
	}

	template<typename T>
	bool transform_hierarchy<T>::dbgui_node(unsigned int node_id, bool display_gizmo)
	{
		const transform_node<T>& node = this->get_node(node_id);
		if(std::find(this->node_free_list.begin(), this->node_free_list.end(), node_id) != this->node_free_list.end())
		{
			// its on the free-list. this is a deleted node. skip
			return true;
		}
		std::string node_name;
		if constexpr(is_printable<T>)
		{
			std::ostringstream oss;
			oss << node.data;
			node_name = "Node \"" + oss.str() + "\"";
		}
		else
		{
			node_name = "Node " + std::to_string(node_id);
		}
		bool ret = true;
		if(ImGui::TreeNode(node_name.c_str()))
		{
			if constexpr(has_dbgui_method<T>)
			{
				node.data.dbgui();
			}
			if(display_gizmo)
			{
				node.local_transform.dbgui();
				tz::mat4 local = node.local_transform.matrix();
				tz::mat4 global = this->get_global_transform(node_id).matrix();
				if(local != tz::mat4::identity())
				{
					ImGui::Text("Local Transform");
					tz::dbgui_model(local);
					ImGui::Text("Global Transform");
					tz::dbgui_model(global);
				}
			}
			for(unsigned int child_idx : node.children)
			{
				if(!this->dbgui_node(child_idx, display_gizmo))
				{
					break;
				}
			}
			if(ImGui::Button("X"))
			{
				this->remove_node(node_id, remove_strategy::patch_children_to_parent);
				ret = false;
			}
			ImGui::TreePop();
		}
		return ret;
	}
}