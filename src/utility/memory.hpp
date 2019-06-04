//
// Created by Harry on 28/05/2019.
//

#ifndef TOPAZ_MEMORY_HPP
#define TOPAZ_MEMORY_HPP
#include <cstddef>
#include <vector>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

/**
 * Manages a pre-allocated but currently unmanaged pool of memory. Although the memory is managed by the class, it does not ever attempt to free the memory; that responsibility remains with the caller.
 * @tparam T - The type of data used in this pool. For pools supporting more than one type, consider a DynamicVariadicMemoryPool.
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
	virtual void zero_all();
	void default_all();
protected:
	T* first;
	T* last;
	std::size_t pool_size;
};

template<typename T>
class AutomaticMemoryPool : public MemoryPool<T>
{
public:
	AutomaticMemoryPool(std::size_t pool_size);
	virtual ~AutomaticMemoryPool();
};

class DynamicVariadicMemoryPool : public MemoryPool<char>
{
public:
	DynamicVariadicMemoryPool(void* begin_address, std::size_t byte_size);
	template<typename T>
	void push_back(T value);
	std::size_t get_size() const;
	std::size_t get_byte_usage() const;
	const std::type_index& get_type_at_index(std::size_t index) const;
	template<typename T>
	T& at(std::size_t index);
	template<typename T>
	const T& at(std::size_t index) const;
	virtual void zero_all() override;
private:
	using MemoryPool<char>::get_element_capacity;
	using MemoryPool<char>::default_all;
	using MemoryPool<char>::operator[];
	void* get_current_offset() const;
	void* get_offset_to_index(std::size_t index) const;

	std::vector<std::type_index> type_format;
	std::unordered_map<std::type_index, std::size_t> type_size_map;
};

class AutomaticDynamicVariadicMemoryPool : public DynamicVariadicMemoryPool
{
public:
	AutomaticDynamicVariadicMemoryPool(std::size_t byte_size);
	virtual ~AutomaticDynamicVariadicMemoryPool();
};

#include "utility/memory.inl"
#endif //TOPAZ_MEMORY_HPP