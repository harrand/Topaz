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

template<class ObjectType>
class RenderableBuffer;

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
    /**
     * Construct a MemoryPool to manage a pre-allocated contiguous range of addresses.
     * @param begin_address - First address in the contiguous range
     * @param pool_size - Number of elements (sizeof T's) that the MemoryPool should accommodate
     */
	MemoryPool(void* begin_address, std::size_t pool_size);
    MemoryPool(const MemoryPool<T>& pool);
    /**
     * Construct a MemoryPool to manage the data of an existing contiguous container, such as an std::vector. The container retains ownership of the memory, this pool simply can read and write to it.
     * @tparam ContiguousContainer - Template class for the contiguous container. The container must be contiguous. So std::vector is fine, but std::set is not. SFINAE is used to prevent this constructor from ever being selected over the MemoryPool copy constructor (as MemoryPool<T> substitutes into ContiguousContainer<T>).
     * @param data - Container value to manage memory of
     */
	template<template<typename> typename ContiguousContainer, typename = typename std::enable_if_t<!std::is_same_v<ContiguousContainer<T>, MemoryPool<T>>>>
	MemoryPool(ContiguousContainer<T>& data);
	iterator begin();
	const_iterator cbegin() const;
	iterator end();
	const_iterator cend() const;
	T& operator[](std::size_t index);
	const T& operator[](std::size_t index) const;
    /**
     * Obtain the maximum number of elements that this pool can store. It is guaranteed to be equal to the pool_size given to the constructor.
     * @return - Number of elements available in the pool
     */
	virtual std::size_t get_element_capacity() const;
    /**
     * Obtain the size (in chars) of this pool.
     * @return - Number of chars that could be stored in this pool
     */
	virtual std::size_t get_byte_capacity() const;
    /**
     * Zero all memory in this pool. Essentially equal to memset.
     */
	virtual void zero_all();
    /**
     * Sets all elements in this pool to be equal to its default value (from a default constructor).
     */
	virtual void default_all();

    template<class ObjectType>
    friend class ::RenderableBuffer;
protected:
    /// First address in the pool.
	T* first;
    /// Last address in the pool.
	T* last;
    /// Number of elements available in the pool.
	std::size_t pool_size;
};

/**
 * Identical to MemoryPool, but is responsible for allocating and de-allocating the memory. Do not use if the memory is pre-allocated, or it will corrupt the heap.
 * @tparam T - Type of element in the pool.
 */
template<typename T>
class AutomaticMemoryPool : public MemoryPool<T>
{
public:
    /**
     * Construct an automatic memory pool of fixed size.
     * @param pool_size - Number of elements in the pool
     */
	AutomaticMemoryPool(std::size_t pool_size);
    /// Deallocates all memory in the pool
	virtual ~AutomaticMemoryPool();
};

template<typename... Ts>
class StaticVariadicMemoryPool : public MemoryPool<char>
{
public:
	StaticVariadicMemoryPool(void* begin_address);
	StaticVariadicMemoryPool(void* begin_address, Ts&&... ts);
    virtual std::size_t get_element_capacity() const override;
	virtual std::size_t get_byte_capacity() const override;
	template<typename T>
	T& get();
	template<typename T>
	const T& get() const;
	virtual void default_all() override;
private:
	using MemoryPool<char>::operator[];
};

template<typename... Ts>
class AutomaticStaticVariadicMemoryPool : public StaticVariadicMemoryPool<Ts...>
{
public:
    AutomaticStaticVariadicMemoryPool();
    AutomaticStaticVariadicMemoryPool(Ts&&... ts);
    virtual ~AutomaticStaticVariadicMemoryPool();
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