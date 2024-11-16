#include "tz/core/hier.hpp"
#include "tz/topaz.hpp"
#include <vector>

namespace tz
{
	struct node_data
	{
		tz::trs local_transform = {};
		node_handle parent = tz::nullhand;
		void* userdata = nullptr;

		std::vector<node_handle> children = {};
	};

	struct hier_data
	{
		std::vector<node_data> nodes = {};
		std::vector<node_handle> free_list = {};
	};

	std::vector<hier_data> hiers = {};

	hier_handle create_hier()
	{
		std::size_t ret = hiers.size();
		hier_data& hier = hiers.emplace_back();

		(void)hier;

		return static_cast<tz::hanval>(ret);
	}

	std::expected<node_handle, tz::error_code> hier_create_node(hier_handle hierh, tz::trs transform, node_handle parent, void* userdata)
	{
		auto& hier = hiers[hierh.peek()];
		if(parent != tz::nullhand)
		{
			auto iter = std::find(hier.free_list.begin(), hier.free_list.end(), parent);
			if(iter != hier.free_list.end())
			{
				UNERR(tz::error_code::invalid_value, "Cannot create a new node with parent {} as this node has previously been destroyed.", parent.peek());
			}
			if(parent.peek() >= hier.nodes.size())
			{
				UNERR(tz::error_code::invalid_value, "Cannot create a new node with parent {} as this is an invalid node", parent.peek());
			}
		}
		std::size_t ret = hier.nodes.size();
		if(hier.free_list.size())
		{
			ret = hier.free_list.back().peek();
			hier.free_list.pop_back();
		}
		else
		{
			hier.nodes.push_back({});
		}
		node_data& new_node = hier.nodes[ret];
		new_node =
		{
			.local_transform = transform,
			.parent = parent,
			.userdata = userdata
		};
		if(parent != tz::nullhand)
		{
			hier.nodes[parent.peek()].children.push_back(static_cast<tz::hanval>(ret));
		}

		return static_cast<tz::hanval>(ret);	
	}

	tz::error_code hier_destroy_node(hier_handle hierh, node_handle node)
	{
		auto& hier = hiers[hierh.peek()];
		if(hier.nodes.size() <= node.peek())
		{
			RETERR(tz::error_code::invalid_value, "invalid node {} in the context of hierarchy {}", node.peek(), hierh.peek());
		}	
		auto iter = std::find(hier.free_list.begin(), hier.free_list.end(), node);
		if(iter != hier.free_list.end())
		{
			RETERR(tz::error_code::invalid_value, "double destroy of node {}", node.peek());
		}
		hier.free_list.push_back(node);
		hier.nodes[node.peek()] = {};

		// delete all child nodes.
		for(node_handle child : hier.nodes[node.peek()].children)
		{
			auto err = hier_destroy_node(hierh, child);
			if(err != tz::error_code::success)
			{
				RETERR(tz::error_code::unknown_error, "During deletion of node {}, failed to destroy child node {} due to {}", node.peek(), child.peek(), tz::error_code_name(err));
			}
		}

		return tz::error_code::success;
	}
}