//
// Created by Harry on 30/05/2019.
//
#include "utility/memory.hpp"

DynamicVariadicMemoryPool::DynamicVariadicMemoryPool(void* begin_address, std::size_t byte_size): MemoryPool<char>(begin_address, byte_size / sizeof(char)), type_format(), type_size_map(){}

std::size_t DynamicVariadicMemoryPool::get_size() const
{
	return this->type_format.size();
}

void* DynamicVariadicMemoryPool::get_current_offset() const
{
	return this->get_offset_to_index(this->get_size() - 1);
}

void* DynamicVariadicMemoryPool::get_offset_to_index(std::size_t index) const
{
	char* address = this->first;
	for(std::size_t i = 0; i < index; i++)
		address += this->type_size_map.at(this->type_format[i]);
	return reinterpret_cast<void*>(address);
}

std::size_t DynamicVariadicMemoryPool::get_byte_usage() const
{
	std::size_t usage = 0;
	for(const auto& info : this->type_format)
		usage += this->type_size_map.at(info);
	return usage;
}

const std::type_index& DynamicVariadicMemoryPool::get_type_at_index(std::size_t index) const
{
	return this->type_format.at(index);
}

void DynamicVariadicMemoryPool::zero_all()
{
	MemoryPool<char>::zero_all();
	this->type_format.clear();
	this->type_size_map.clear();
}

AutomaticDynamicVariadicMemoryPool::AutomaticDynamicVariadicMemoryPool(std::size_t byte_size) : DynamicVariadicMemoryPool(std::malloc(byte_size), byte_size){}

AutomaticDynamicVariadicMemoryPool::~AutomaticDynamicVariadicMemoryPool()
{
	std::free(this->first);
}