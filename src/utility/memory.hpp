//
// Created by Harry on 28/05/2019.
//

#ifndef TOPAZ_MEMORY_HPP
#define TOPAZ_MEMORY_HPP
#include <cstddef>
#include <vector>
#include <typeindex>
#include <typeinfo>

/**
 * Manages a pre-allocated but currently unmanaged pool of memory.
 * @tparam T - The type of data used in this pool. For pools supporting more than one type, consider a VariadicMemoryPool.
 */
template<typename T>
class MemoryPool
{
public:
	class iterator : public std::iterator<std::random_access_iterator_tag, T>
	{
	public:
		iterator(std::size_t index, T* value);
		void operator++();
		void operator--();
		T& operator*() const;
		bool operator!=(const iterator& rhs) const;
	private:
		std::size_t index;
		T* value;
	};
	class const_iterator : public std::iterator<std::random_access_iterator_tag, const T>
	{
	public:
		const_iterator(std::size_t index, const T* value);
		void operator++();
		void operator--();
		const T& operator*() const;
		bool operator!=(const const_iterator& rhs) const;
	private:
		std::size_t index;
		const T* value;
	};
	MemoryPool(void* begin_address, std::size_t pool_size);
	template<template<typename> typename ContiguousContainer>
	MemoryPool(ContiguousContainer<T>& data);
	iterator begin();
	const_iterator cbegin() const;
	iterator end();
	const_iterator cend() const;
	T& operator[](std::size_t index);
	const T& operator[](std::size_t index) const;
	std::size_t get_element_capacity() const;
	std::size_t get_byte_capacity() const;
	void zero_all();
	void default_all();
private:
	T* first;
	T* last;
	std::size_t pool_size;
};

#include "utility/memory.inl"
#endif //TOPAZ_MEMORY_HPP