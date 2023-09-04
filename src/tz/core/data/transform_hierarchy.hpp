#ifndef TZ_CORE_DATA_TRANSFORM_HIERARCHY_HPP
#define TZ_CORE_DATA_TRANSFORM_HIERARCHY_HPP
#include "tz/core/data/trs.hpp"
#include "tz/core/data/handle.hpp"
#include <optional>
#include <cstddef>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * Represents a node within a transform hierarchy.
	 **/
	template<typename T>
	struct transform_node
	{
		/// User-data. You can use this for whatever you wish. Must be at least moveable.
		mutable T data;
		/// Local transform, relative to its parent. If there is no parent, then this represents the global transform aswell.
		mutable tz::trs local_transform = {};
		/// Index of the node that is this node's parent, or nullopt if this node has no parent.
		std::optional<unsigned int> parent = std::nullopt;
		/// List of all indices representing this node's children.
		std::vector<unsigned int> children = {};
	};

	/**
	 * @ingroup tz_core
	 * Represents a hierarchy of 3D transformations, with a payload applied.
	 * Useful to represent a scene graph, or nodes for 3D skeletal animation.
	 **/
	template<typename T = void*>
	class transform_hierarchy
	{
	public:
		/// Node type
		using node = transform_node<T>;
		/// Create a new, empty hierarchy.
		transform_hierarchy() = default;
		/**
		 * Retrieves the number of nodes within the hierarchy.
		 * @return Number of nodes in the hierarchy.
		 **/
		std::size_t size() const;
		/**
		 * Query as to whether the hierarchy has nodes.
		 * @return True if `this->size() == 0`, otherwise false.
		 **/
		bool empty() const{return this->size() == 0;}
		/**
		 * Remove all nodes from the hierarchy, emptying it.
		 * @note Invalidates all node indices.
		 **/
		void clear();
		/**
		 * Retrieve a vector containing indices corresponding to all the root nodes.
		 * @return Vector containing indices of root nodes (nodes without a parent).
		 **/
		std::vector<unsigned int> get_root_node_ids() const;

		// returns id of the new node.
		/**
		 * Add a new node to the hierarchy.
		 * @param local_transform Local transform of the node, local to its parent (if it has one). You can change this at any time.
		 * @param data Data payload of the node. You can change this at any time.
		 * @param parent Optional index of the parent. If the node has no parent, omit this parameter, or set it to `nullopt`.
		 * @return Index of the newly-created node.
		 **/
		unsigned int add_node(tz::trs local_transform = {}, T data = {}, std::optional<unsigned int> parent = std::nullopt);
		// returns offset to be applied to the previous hierarchy's set of nodes to get their corresponding node ids
		// within this hierarchy
		/**
		 * Add all of the nodes of another hierarchy to this hierarchy, preserving structure.
		 * @param tree Hierarchy whose nodes should be copied into this hierarchy.
		 * @note The root nodes of the provided hierarchy will remain root nodes in this hierarchy. To attach these to an existing node, see @ref add_hierarchy_onto
		 * @return Integer representing the offset that should be applied to each index of the previous tree. i.e `tree.get_node(i)` corresponds to `this->get_node(i+n)`, where `n` is the return value of this method.
		 **/
		unsigned int add_hierarchy(const transform_hierarchy<T>& tree);
		/**
		 * Add all of the nodes of another hierarchy onto a particular node of this hierarchy, preserving structure.
		 * @param tree Hierarchy whose nodes should be copied into this hierarchy.
		 * @param node_id A valid node index for *this* hierarchy, where all the nodes of the provided hierarchy will be attached to.
		 * @note This is similar to @ref add_hierarchy but root nodes of the provided hierarchy will be attached to the targeted node instead.
		 * @return Integer representing the offset that should be applied to each index of the previous tree. i.e `tree.get_node(i)` corresponds to `this->get_node(i+n)`, where `n` is the return value of this method.
		 */
		unsigned int add_hierarchy_onto(const transform_hierarchy<T>& tree, unsigned int node_id);
		/**
		 * Take a copy of a particular node, and create a new hierarchy with that as the only root node.
		 * @param id Index corresponding to the node which should become the root node of the resultant hierarchy.
		 * @return Hierarchy, where the node corresponding to `id` is the root node, aswell as copies all of its children and descendants.
		 * @note The global transform of the node is preserved. That is - the global transform of the node within this hierarchy will be equal to the local transform of the root node of the new hierarchy.
		 **/
		transform_hierarchy<T> export_node(unsigned int id) const;
		/**
		 * Retrieve the node corresponding to the given index.
		 * @param id Index corresponding to the node to retrieve.
		 * @return Node corresponding to the provided index.
		 * @pre `id` < `this->size()`, otherwise the behaviour is undefined.
		 **/
		const transform_node<T>& get_node(unsigned int id) const;
		/**
		 * Retrieve the global transform of the node corresponding to the provided id.
		 * @param id Index corresponding to the node to calculate the global transform.
		 * @return TRS representing the global transform of the `id`'th node.
		 **/
		tz::trs get_global_transform(unsigned int id) const;
		/**
		 * Invoke a callback for each child of the node corresponding to `id`.
		 * @param id Index corresponding to the node whose children to iterate over.
		 * @param callback Callback to be invoked with the current child's node index. Guaranteed to be invoked on the initial caller thread.
		 **/
		void iterate_children(unsigned int id, tz::action<unsigned int> auto callback) const;
		/**
		 * Invoke a callback for each descendent of the node corresponding to `id`. A descendent is a node that is either a child, grandchild etc... of the provided node.
		 * @param id Index corresponding to the node whose descendants to iterate over.
		 * @param callback Callback to be invoked with the current descendant's node index. Guaranteed to be invoked on the initial caller thread.
		 **/
		void iterate_descendants(unsigned int id, tz::action<unsigned int> auto callback) const;
		// invoke callback for each ancestor (parent, their parent, etc...) of the node `id`
		/**
		 * Invoke a callback for each ancestor of the node corresponding to `id`. An ancestor is a node that is either a parent, grandparent etc... of the provided node.
		 * @param id Index corresponding to the node whose ancestors to iterate over.
		 * @param callback Callback to be invoked with the current ancestor's node index. Guaranteed to be invoked on the initial caller thread.
		 **/
		void iterate_ancestors(unsigned int id, tz::action<unsigned int> auto callback) const;
		/**
		 * Invoke a callback for each node within the hierarchy, in-order. Depth-first traversal.
		 * @param callback Callback to be invoked exactly once for each node in the hierarchy. Guaranteed to be invoked on the initial caller thread.
		 **/
		void iterate_nodes(tz::action<unsigned int> auto callback) const;

		void dbgui();
	private:
		void dbgui_node(unsigned int node_id);
		std::vector<transform_node<T>> nodes = {};
	};
}

#include "tz/core/data/transform_hierarchy.inl"
#endif // TZ_CORE_DATA_TRANSFORM_HIERARCHY_HPP