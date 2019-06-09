#include <cstring>
#include "core/topaz.hpp"

template<typename T>
MemoryPool<T>::iterator::iterator(std::size_t index, T* value): index(index), value(value){}

template<typename T>
void MemoryPool<T>::iterator::operator++()
{
	this->index++;
	this->value++;
}

template<typename T>
void MemoryPool<T>::iterator::operator--()
{
	this->index--;
	this->value--;
}

template<typename T>
T& MemoryPool<T>::iterator::operator*() const
{
	return *this->value;
}

template<typename T>
bool MemoryPool<T>::iterator::operator!=(const iterator& rhs) const
{
	return this->index != rhs.index;
}

template<typename T>
MemoryPool<T>::const_iterator::const_iterator(std::size_t index, const T* value): index(index), value(value){}

template<typename T>
void MemoryPool<T>::const_iterator::operator++()
{
	this->index++;
	this->value++;
}

template<typename T>
void MemoryPool<T>::const_iterator::operator--()
{
	this->index--;
	this->value--;
}

template<typename T>
const T& MemoryPool<T>::const_iterator::operator*() const
{
	return *this->value;
}

template<typename T>
bool MemoryPool<T>::const_iterator::operator!=(const const_iterator& rhs) const
{
	return this->index != rhs.index;
}

template<typename T>
MemoryPool<T>::MemoryPool(void *begin_address, std::size_t pool_size): first(reinterpret_cast<T*>(begin_address)), last(reinterpret_cast<T*>(begin_address) + pool_size - 1), pool_size(pool_size){}

template<typename T>
MemoryPool<T>::MemoryPool(const MemoryPool<T>& pool): MemoryPool<T>(pool.begin_address, pool.pool_size){}

template<typename T>
template<template<typename> typename ContiguousContainer>
MemoryPool<T>::MemoryPool(ContiguousContainer<T>& data): MemoryPool(data.data(), data.size()){}

template<typename T>
typename MemoryPool<T>::iterator MemoryPool<T>::begin()
{
	return {0, this->first};
}

template<typename T>
typename MemoryPool<T>::const_iterator MemoryPool<T>::cbegin() const
{
	return {0, this->first};
}

template<typename T>
typename MemoryPool<T>::iterator MemoryPool<T>::end()
{
	return {this->pool_size, this->last};
}

template<typename T>
typename MemoryPool<T>::const_iterator MemoryPool<T>::cend() const
{
	return {this->pool_size, this->last};
}

template<typename T>
T& MemoryPool<T>::operator[](std::size_t index)
{
	topaz_assert(index < pool_size, "MemoryPool<T> accessed at index ", index, " which is out of range of this pool (size ", this->pool_size, ").");
	return *(this->first + index);
}

template<typename T>
const T& MemoryPool<T>::operator[](std::size_t index) const
{
	topaz_assert(index < this->pool_size, "MemoryPool<T> accessed at index ", index, " which is out of range of this pool (size ", this->pool_size, ").");
	return *(this->first + index);
}

template<typename T>
std::size_t MemoryPool<T>::get_element_capacity() const
{
	return this->pool_size;
}

template<typename T>
std::size_t MemoryPool<T>::get_byte_capacity() const
{
	return this->pool_size * sizeof(T);
}

template<typename T>
void MemoryPool<T>::zero_all()
{
	std::memset(this->first, 0, this->get_byte_capacity());
}

template<typename T>
void MemoryPool<T>::default_all()
{
	for(T& t : *this)
		t = T{};
}

template<typename T>
AutomaticMemoryPool<T>::AutomaticMemoryPool(std::size_t pool_size): MemoryPool<T>(std::malloc(sizeof(T) * pool_size), pool_size){}

template<typename T>
AutomaticMemoryPool<T>::~AutomaticMemoryPool()
{
	std::free(this->first);
}

template<typename T>
void DynamicVariadicMemoryPool::push_back(T value)
{
	std::type_index index = typeid(value);
	this->type_format.push_back(index);
	if(this->type_size_map.find(index) == this->type_size_map.end())
		this->type_size_map[index] = sizeof(T);
	// Actually put it in the pool
	*reinterpret_cast<T*>(this->get_current_offset()) = value;
}

template<typename T>
T& DynamicVariadicMemoryPool::at(std::size_t index)
{
	return *reinterpret_cast<T*>(this->get_offset_to_index(index));
}

template<typename T>
const T& DynamicVariadicMemoryPool::at(std::size_t index) const
{
	return *reinterpret_cast<T*>(this->get_offset_to_index(index));
}