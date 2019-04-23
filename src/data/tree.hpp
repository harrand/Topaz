//
// Created by Harrand on 15/04/2019.
//

#ifndef TOPAZ_TREE_HPP
#define TOPAZ_TREE_HPP
#include <cstddef>
#include <array>
#include <memory>

/**
 * Represent a tree of any dimensions, holding any type.
 * Data members remain protected without any utility functions. This is so any concrete data type (such as a binary search tree) can implement their own functionality as they should.
 * @tparam T - Type of each node of the tree
 * @tparam dimension - Number of children per tree (for example, a BinaryTree is 2, a QuadTree is 4)
 */
template<typename T, std::size_t dimension>
class Tree
{
public:
	/**
	 * Construct an empty tree.
	 */
	Tree();
	/**
	 * Retrieve the parent node, if there is one.
	 * @return - Pointer to parent node if it exists. Otherwise nullptr
	 */
	const T* get_parent() const;
	/**
	 * Retrieve a child node at the given index, if there is one.
	 * @param child_index - Index of the child to edit (should be between 0-7)
	 * @return - Pointer to given child node, if there is one. Otherwise nullptr
	 */
	const T* get_child(std::size_t child_index) const;
protected:
	/// Stores the location of the parent (non-owning).
	T* parent;
	/// Stores all child nodes (owning).
	std::array<std::unique_ptr<T>, dimension> children;
};

template<typename T>
using BinaryTree = Tree<T, 2>;

template<typename T>
using QuadTree = Tree<T, 4>;

template<typename T>
using Octree = Tree<T, 8>;

#include "tree.inl"
#endif //TOPAZ_TREE_HPP
