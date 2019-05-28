#include <cstring>

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