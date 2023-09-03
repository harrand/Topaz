#include "tz/core/data/transform_hierarchy.hpp"
#include "tz/core/debug.hpp"

void empty_hierarchy_tests()
{
	tz::transform_hierarchy empty;
	tz::assert(empty.empty());
}

void root_nodes_local_equals_global()
{
	tz::transform_hierarchy t;
	unsigned int root1 = t.add_node();
	unsigned int root2 = t.add_node();
	unsigned int child1 = t.add_node({}, {}, root1);
	unsigned int child11 = t.add_node({}, {}, child1);
	unsigned int child2 = t.add_node({}, {}, root2);
	
	tz::assert(t.get_root_node_ids().size() == 2, "Expected 2 root nodes, but got %zu", t.get_root_node_ids().size());
	tz::assert(t.get_root_node_ids() == std::vector<unsigned int>{0u, 1u} || t.get_root_node_ids() == std::vector<unsigned int>{1u, 0u});
}

void parent_moves_child()
{
	tz::transform_hierarchy t;
	unsigned int parent = t.add_node();
	unsigned int child = t.add_node({.translate = {1.0f, 0.0f, 0.0f}}, {}, parent);
	tz::assert(t.get_global_transform(child).translate == tz::vec3{1.0f, 0.0f, 0.0f});
	t.get_node(child).local_transform.translate[0] += 1.0f;
	tz::assert(t.get_global_transform(child).translate == tz::vec3{2.0f, 0.0f, 0.0f});
	t.get_node(parent).local_transform.translate[0]+= 5.0f;
	tz::assert(t.get_global_transform(child).translate == tz::vec3{7.0f, 0.0f, 0.0f});
}

void combine_hierarchies()
{
	tz::transform_hierarchy<std::string> lhs;
	unsigned int lhsroot = lhs.add_node({}, "LHS ROOT");

	tz::transform_hierarchy<std::string> rhs;
	unsigned int rhsroot = rhs.add_node({}, "RHS ROOT");
	unsigned int rhschild = rhs.add_node({}, "RHS CHILD", rhsroot);
	tz::assert(rhs.get_node(rhschild).parent == rhsroot);

	unsigned int offset = lhs.add_hierarchy(rhs);
	tz::assert(lhs.get_node(lhsroot).data == "LHS ROOT");
	tz::assert(lhs.get_node(rhsroot + offset).data == "RHS ROOT");
	tz::assert(lhs.get_node(rhschild + offset).data == "RHS CHILD");
	tz::assert(lhs.get_node(rhschild + offset).parent == (rhsroot + offset));
}

void combine_hierarchy_into_node()
{
	tz::transform_hierarchy<std::string> lhs;
	unsigned int lhsroot = lhs.add_node({}, "LHS ROOT");
	unsigned int lhschild = lhs.add_node({}, "LHS CHILD", lhsroot);

	tz::transform_hierarchy<std::string> rhs;
	unsigned int rhsroot = rhs.add_node({}, "RHS ROOT");
	unsigned int rhschild = rhs.add_node({}, "RHS CHILD", rhsroot);
	tz::assert(!rhs.get_node(rhsroot).parent.has_value());
	tz::assert(rhs.get_node(rhschild).parent == rhsroot);

	unsigned int offset = lhs.add_hierarchy_onto(rhs, lhschild);
	tz::assert(lhs.get_node(lhsroot).data == "LHS ROOT");
	tz::assert(lhs.get_node(rhsroot + offset).data == "RHS ROOT");
	tz::assert(lhs.get_node(rhschild + offset).data == "RHS CHILD");
	tz::assert(lhs.get_node(rhschild + offset).parent == (rhsroot + offset));
	tz::assert(lhs.get_node(rhsroot + offset).parent.has_value());
	tz::assert(lhs.get_node(rhsroot + offset).parent == lhschild);
}

int main()
{
	empty_hierarchy_tests();
	root_nodes_local_equals_global();
	parent_moves_child();
	combine_hierarchies();
	combine_hierarchy_into_node();
}