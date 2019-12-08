//
// Created by Harrand on 15/04/2019.
//

#ifndef TOPAZ_TREE_HPP
#define TOPAZ_TREE_HPP
#include <cstddef>
#include <array>
#include <memory>

template<typename T>
class GenericTree
{
public:
	virtual ~GenericTree() = default;
	virtual const T* get_parent() const = 0;
	virtual const T* get_child(std::size_t child_index) const = 0;
};

/**
 * Represent a tree of any dimensions, holding any type.
 * Data members remain protected without any utility functions. This is so any concrete data type (such as a binary search tree) can implement their own functionality as they should.
 * @tparam T - Type of each node of the tree
 * @tparam dimension - Number of children per tree (for example, a BinaryTree is 2, a QuadTree is 4)
 */
template<typename T, std::size_t dimension>
class Tree : public GenericTree<T>
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
	virtual const T* get_parent() const override;
	/**
	 * Retrieve a child node at the given index, if there is one.
	 * @param child_index - Index of the child to edit (should be between 0-7)
	 * @return - Pointer to given child node, if there is one. Otherwise nullptr
	 */
	virtual const T* get_child(std::size_t child_index) const override;
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

template<typename T>
class DynamicTree : public GenericTree<T>
{
public:
	DynamicTree();
	DynamicTree(const T* parent);
	virtual const T* get_parent() const override;
	virtual const T* get_child(std::size_t child_index) const override;
	template<typename... Args>
	bool set_child(std::size_t child_index, Args&&... args);
	std::size_t get_dimensions() const;
private:
	T* parent;
	std::vector<std::unique_ptr<T>> children;
};

#include "tree.inl"
#endif //TOPAZ_TREE_HPP
