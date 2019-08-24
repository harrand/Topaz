#include <cstring>
#include "core/topaz.hpp"

template<typename PoolMarking>
void PoolMarker<PoolMarking>::mark(PoolMarker::MemoryRegion region, PoolMarking marking)
{
	this->marked_regions.emplace(marking, std::move(region));
}

template<typename PoolMarking>
void PoolMarker<PoolMarking>::unmark(PoolMarking marking)
{
	for(auto it : this->marked_regions)
	{
		if(it.first == marking)
			this->marked_regions.erase(it++);
		else
			++it;
	}
}

template<typename PoolMarking>
void PoolMarker<PoolMarking>::unmark(PoolMarker<PoolMarking>::MemoryRegion region)
{
	char* begin = reinterpret_cast<char*>(region.first);
	char* end = reinterpret_cast<char*>(region.second);
	topaz_assert(std::less_equal<char*>{}(begin, end), "PoolMarking::unmark(...): Given region needs to be coherent (begin < end)");
	for(char* i = begin; i < end; i++)
	{
		this->unmark_address(reinterpret_cast<void*>(i));
	}
}

template<typename PoolMarking>
std::optional<PoolMarking> PoolMarker<PoolMarking>::get_mark(const void* address) const
{
	auto within_region = [address](const PoolMarker<PoolMarking>::MemoryRegion& region)->bool
	{
		// Assume first is smaller than second.
		std::less_equal<const void*> less_func;
		return less_func(region.first, address) && less_func(address, region.second);
	};
	for(const auto& [marking, region] : this->marked_regions)
	{
		if(within_region(region))
			return {marking};
	}
	return {std::nullopt};
}

template<typename PoolMarking>
std::vector<PoolMarking> PoolMarker<PoolMarking>::get_marks(const PoolMarker::MemoryRegion& region) const
{
	std::vector<PoolMarking> marks;
	char* begin = reinterpret_cast<char*>(region.first);
	char* end = reinterpret_cast<char*>(region.second);
	topaz_assert(std::less_equal<char*>{}(begin, end), "PoolMarking::get_marks(...): Given region needs to be coherent (begin < end)");
	for(char* i = begin; i < end; i++)
	{
		auto marking = this->get_mark(reinterpret_cast<void*>(i));
		if(marking.has_value())
			marks.push_back(marking.value());
	}
	return marks;
}

template<typename PoolMarking>
void PoolMarker<PoolMarking>::unmark_address(const void *address)
{
	auto within_region = [address](const PoolMarker<PoolMarking>::MemoryRegion& region)->bool
	{
		// Assume first is smaller than second.
		std::less_equal<const void*> less_func;
		return less_func(region.first, address) && less_func(address, region.second);
	};
	for(auto it : this->marked_regions)
	{
		PoolMarker<PoolMarking>::MemoryRegion region = it.first;
		PoolMarking marking = it.second;
		if(within_region(region))
		{
			// Need to split the region.
			char* begin = reinterpret_cast<char*>(region.first);
			char* end = reinterpret_cast<char*>(region.second);
			for(char* i = begin; i < end; i++)
			{
				MemoryRegion a = std::make_pair(region.first, nullptr);
				MemoryRegion b = std::make_pair(nullptr, region.second);
				if(reinterpret_cast<void*>(i) == address)
				{
					a.second = i;
					b.first = i + 1;
				}

				// We know how to partition, now we can actually do it.
				this->marked_regions.erase(it++);
				this->marked_regions[marking] = a;
				this->marked_regions[marking] = b;

			}
		}
	}
}

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
MemoryPool<T>::MemoryPool(const MemoryPool<T>& pool): MemoryPool<T>(pool.first, pool.pool_size){}

template<typename T>
template<template<typename> typename ContiguousContainer, typename>
MemoryPool<T>::MemoryPool(ContiguousContainer<T>& data): MemoryPool(data.data(), data.size()){}

template<typename T>
template<template<typename> typename ContiguousContainer, typename>
MemoryPool<T>& MemoryPool<T>::operator=(const ContiguousContainer<T>& data)
{
	topaz_assert(data.size() <= this->get_element_capacity(), "MemoryPool<T>& MemoryPool<T>::operator=(...): Assigning from ContiguousContainer failed, because the ContiguousContainer size (", data.size(), ") is larger than this allocated pool (", this->get_element_capacity(), ").");
	for(std::size_t i = 0; i < data.size(); i++)
		(*this)[i] = data[i];
	return *this;
}

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
void MemoryPool<T>::mark(std::size_t index, MarkerType mark)
{
    MemoryPool<T>::mark_value((*this)[index], mark);
}

template<typename T>
void MemoryPool<T>::mark_value(const T& element, MarkerType mark)
{
    for(const auto& el : *this)
    {
        if (el == element)
            PoolMarker::mark({&el, &el}, mark);
    }
}

template<typename T>
void MemoryPool<T>::mark_range(MemoryPool<T>::iterator begin, MemoryPool<T>::iterator end, MarkerType mark)
{
    for(auto it = begin; it != end; it++)
        MemoryPool<T>::mark(*it, mark);
}

template<typename T>
void MemoryPool<T>::mark_indices(std::size_t begin_index, std::size_t end_index, MarkerType mark)
{
    for(std::size_t i = begin_index + 1; i <= end_index; ++i)
        MemoryPool<T>::mark_value((*this)[i], mark);
}

template<typename T>
void MemoryPool<T>::unmark(std::size_t index)
{
    MemoryPool<T>::unmark_value((*this)[index]);
}

template<typename T>
void MemoryPool<T>::unmark_value(const T& element)
{
    for(const auto& el : *this)
    {
        if (el == element)
            PoolMarker::unmark({&el, &el});
    }
}

template<typename T>
void MemoryPool<T>::unmark_range(MemoryPool<T>::iterator begin, MemoryPool<T>::iterator end)
{
    for(auto it = begin; it != end; it++)
        MemoryPool<T>::unmark(*it);
}

template<typename T>
void MemoryPool<T>::unmark_indices(std::size_t begin_index, std::size_t end_index)
{
    for(std::size_t i = begin_index; i < end_index; i++)
        MemoryPool<T>::unmark_value((*this)[i]);
}

template<typename T>
std::optional<PoolMarker<>::MarkerType> MemoryPool<T>::get_mark(std::size_t index) const
{
    return MemoryPool<T>::get_value_mark((*this)[index]);
}

template<typename T>
std::optional<PoolMarker<>::MarkerType> MemoryPool<T>::get_value_mark(const T& element) const
{
    return PoolMarker::get_mark(&element);
}

template<typename T>
bool MemoryPool<T>::empty() const
{
	return this->get_element_capacity() == 0;
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
AutomaticMemoryPool<T>::AutomaticMemoryPool(const MemoryPool<T>& copy): AutomaticMemoryPool<T>(copy.get_element_capacity())
{
	for(std::size_t i = 0; i < copy.get_element_capacity(); i++)
	{
		(*this)[i] = copy[i];
	}
}

template<typename T>
AutomaticMemoryPool<T>::~AutomaticMemoryPool()
{
	std::free(this->first);
}

template<typename... Ts>
StaticVariadicMemoryPool<Ts...>::StaticVariadicMemoryPool(void* begin_address): MemoryPool<char>(begin_address, this->get_byte_capacity()){}

template<typename... Ts>
StaticVariadicMemoryPool<Ts...>::StaticVariadicMemoryPool(void* begin_address, Ts&&... ts): StaticVariadicMemoryPool<Ts...>(begin_address)
{
	*reinterpret_cast<std::tuple<Ts...>*>(this->first) = {std::forward<Ts>(ts)...};
}

template<typename... Ts>
StaticVariadicMemoryPool<Ts...>::StaticVariadicMemoryPool(MemoryPool<char>&& fixed_pool): StaticVariadicMemoryPool<Ts...>(&*fixed_pool.begin())
{
	topaz_assert(fixed_pool.get_byte_capacity() == this->get_byte_capacity(), "StaticVariadicMemoryPool<Ts...>::StaticVariadicMemoryPool(MemoryPool<char>&&): The byte-pool provided was of size ", fixed_pool.get_byte_capacity(), ", which does not fit properly into an SVMPool of this format of size ", this->get_byte_capacity(), ".");
}

template<typename... Ts>
std::size_t StaticVariadicMemoryPool<Ts...>::get_element_capacity() const
{
	return sizeof...(Ts);
}

template<typename... Ts>
std::size_t StaticVariadicMemoryPool<Ts...>::get_byte_capacity() const
{
	return (sizeof(Ts) + ...);
}

template<typename... Ts>
template<typename T>
T& StaticVariadicMemoryPool<Ts...>::get()
{
	return std::get<T>(*reinterpret_cast<std::tuple<Ts...>*>(this->first));
}

template<typename... Ts>
template<typename T>
const T& StaticVariadicMemoryPool<Ts...>::get() const
{
	return std::get<T>(*reinterpret_cast<std::tuple<Ts...>*>(this->first));
}

template<typename... Ts>
template<std::size_t I>
decltype(auto) StaticVariadicMemoryPool<Ts...>::get()
{
	return std::get<I>(*reinterpret_cast<std::tuple<Ts...>*>(this->first));
}

template<typename... Ts>
template<std::size_t I>
decltype(auto) StaticVariadicMemoryPool<Ts...>::get() const
{
	return std::get<I>(*reinterpret_cast<std::tuple<Ts...>*>(this->first));
}

template<typename... Ts>
void StaticVariadicMemoryPool<Ts...>::default_all()
{
	// Need to value initialise all objects
	*reinterpret_cast<std::tuple<Ts...>*>(this->first) = {};
}

template<typename... Ts>
AutomaticStaticVariadicMemoryPool<Ts...>::AutomaticStaticVariadicMemoryPool(): StaticVariadicMemoryPool<Ts...>(std::malloc(this->get_byte_capacity())){}

template<typename... Ts>
AutomaticStaticVariadicMemoryPool<Ts...>::AutomaticStaticVariadicMemoryPool(const StaticVariadicMemoryPool<Ts...>& copy): AutomaticStaticVariadicMemoryPool()
{
    *reinterpret_cast<std::tuple<Ts...>*>(this->first) = *reinterpret_cast<const std::tuple<Ts...>*>(&*copy.cbegin());
}

template<typename... Ts>
AutomaticStaticVariadicMemoryPool<Ts...>::AutomaticStaticVariadicMemoryPool(Ts&&... ts): StaticVariadicMemoryPool<Ts...>(std::malloc(this->get_byte_capacity()), std::forward<Ts>(ts)...){}

template<typename... Ts>
AutomaticStaticVariadicMemoryPool<Ts...>::~AutomaticStaticVariadicMemoryPool()
{
	std::free(this->first);
}

template<typename T>
DynamicVariadicMemoryPool::DynamicVariadicMemoryPool(MemoryPool<T>&& pool, std::size_t inserted_size): DynamicVariadicMemoryPool(reinterpret_cast<char*>(&*pool.begin()), pool.get_byte_capacity())
{
	std::type_index index = typeid(T);
	for(std::size_t i = 0; i < inserted_size; i++)
		this->type_format.push_back(index);
	this->type_size_map[index] = sizeof(T);
}

template<typename T>
void DynamicVariadicMemoryPool::push_back(T value)
{
	std::type_index index = typeid(value);
	this->type_format.push_back(index);
	if(this->type_size_map.find(index) == this->type_size_map.end())
		this->type_size_map[index] = sizeof(T);
	// Actually put it in the pool. Assert first that we can do so.
	void* current_offset = this->get_current_offset();
    if constexpr(tz::is_debug_mode)
    {
        auto last_ptr = reinterpret_cast<void*>(this->last);
        auto after_push_offset = reinterpret_cast<void*>(reinterpret_cast<char*>(current_offset) + sizeof(T));
        auto diff = 1 + reinterpret_cast<std::ptrdiff_t>(reinterpret_cast<char*>(last_ptr) - reinterpret_cast<char*>(after_push_offset));
        topaz_assert(diff >= 0, "DynamicVariadicMemoryPool::push_back(...): Attempted to push back when doing so would go out of bounds:\nBoundary = ", last_ptr, "\nCurrent End = ", current_offset, "\nEnd after push_back = ", after_push_offset, "\nDiff = ", diff, " (" , -diff, " bytes out of bounds)\n");
    }
	*reinterpret_cast<T*>(current_offset) = value;
}

template<typename T>
T& DynamicVariadicMemoryPool::at(std::size_t index)
{
    topaz_assert(this->get_size() > index, "DynamicVariadicMemoryPool::at<T>(...): Index passed (", index, ") is out of range for the pool of size ", this->get_size(), ", it must be between 0-", this->get_size() - 1, ".\n");
    topaz_assert(std::type_index(typeid(T)) == this->get_type_at_index(index), "DynamicVariadicMemoryPool::at<T>(...): Template argument T (", typeid(T).name(), ") does not match the actual type at index ", index, " (which is ", this->get_type_at_index(index).name(), ")\n");
	return *reinterpret_cast<T*>(this->get_offset_to_index(index));
}

template<typename T>
const T& DynamicVariadicMemoryPool::at(std::size_t index) const
{
	topaz_assert(this->get_size() > index, "DynamicVariadicMemoryPool::at<T>(...): Index passed (", index, ") is out of range for the pool of size ", this->get_size(), ", it must be between 0-", this->get_size() - 1, ".\n");
	topaz_assert(std::type_index(typeid(T)) == this->get_type_at_index(index), "DynamicVariadicMemoryPool::at<T>(...): Template argument T (", typeid(T).name(), ") does not match the actual type at index ", index, " (which is ", this->get_type_at_index(index).name(), ")\n");
	return *reinterpret_cast<T*>(this->get_offset_to_index(index));
}

namespace tz::utility::memory
{
	template<typename T, typename... Ts>
	std::pair<SVMPool<Ts...>, MemoryPool<T>> partition(MemoryPool<T> initial_pool)
	{
		std::size_t svm_size = (sizeof(Ts) + ...);
		topaz_assert(initial_pool.get_byte_capacity() >= svm_size, "tz::utility::memory::partition<SVMPool, MemoryPool>(MemoryPool<T>): Cannot partition a pool when the daughter SVMPool would be larger than the initial pool.");
		topaz_assert(initial_pool.get_element_capacity() > 0, "tz::utility::memory::partition<SVMPool, MemoryPool>(MemoryPool<T>): Initial pool must contain at least one POD.");
		char* offset = reinterpret_cast<char*>(&initial_pool[0]) + svm_size;
		std::size_t remainder_size = static_cast<std::size_t>(reinterpret_cast<char*>(&initial_pool[initial_pool.get_element_capacity() - 1]) - offset) / sizeof(T) + 1;
		return {{&initial_pool[0]}, {offset, remainder_size}};
	}
}