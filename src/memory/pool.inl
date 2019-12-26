#include "core/debug/assert.hpp"
#include <cstddef>

namespace tz::mem
{	
	template<typename T>
	UniformPool<T>::UniformPool(void* begin, void* end): UniformPool<T>(begin, byte_distance(begin, end)){}
	
	template<typename T>
	UniformPool<T>::UniformPool(Block block): UniformPool<T>(block.begin, block.end){}

	template<typename T>
	UniformPool<T>::UniformPool(void* begin, std::size_t size_bytes): begin(begin), size_bytes(size_bytes){}
	
	template<typename T>
	std::size_t UniformPool<T>::size() const
	{
		std::size_t sz = 0;
		for(const auto& b : this->object_mask)
		{
			if(b)
				sz++;
		}
		return sz;
	}
	
	template<typename T>
	std::size_t UniformPool<T>::capacity() const
	{
		static_assert(sizeof(T) != 0, "UniformPool<T> uses underlying type with sizeof 0. This is completely wrong.");
		return this->size_bytes / sizeof(T);
	}
	
	template<typename T>
	bool UniformPool<T>::full() const
	{
		return this->size() == this->capacity();
	}
	
	template<typename T>
	bool UniformPool<T>::empty() const
	{
		return this->size() == 0;
	}
	
	template<typename T>
	std::size_t UniformPool<T>::capacity_bytes() const
	{
		return this->capacity() * sizeof(T);
	}
	
	template<typename T>
	void UniformPool<T>::set(std::size_t index, T t)
	{
		topaz_assert(index < this->capacity(), "UniformPool<T>::set(index, ...): Setting at index", index, " is beyond the capacity of ", this->capacity());
		T* ptr = this->at(index);
		// This element already exists. Invoke its destructor first.
		if(this->is_object(index))
		{
			ptr->~T();
		}
		// Now create our object here. No need to do anything with the return result because we are laundering memory.
		new (ptr) T{t};
		// Make sure our mask is long enough.
		this->object_mask.resize(std::max(index + 1, this->object_mask.capacity()), false);
		// And obviously we now have an object here.
		this->object_mask[index] = true;
	}
	
	template<typename T>
	void UniformPool<T>::set(std::size_t index, std::nullptr_t)
	{
		if(index >= this->capacity())
		{
			// Assert + Early out as the unit-test will prevent these from being fatal.
			topaz_assert(false, "UniformPool<T>::set(index, null): Erasing at index", index, " is beyond the capacity of ", this->capacity());
			// In that specific case, we don't want to go on and invoke UB so we will early-out here.
			return;
		}
		if(this->is_object(index))
		{
			// if there was something here, we should invoke its destructor!
			this->at(index)->~T();
		}
		// Make sure our mask is long enough.
		this->object_mask.resize(std::max(index + 1, this->object_mask.capacity()), false);
		// And obviously we no longer have an object here.
		this->object_mask[index] = false;
	}
	
	template<typename T>
	void UniformPool<T>::erase(std::size_t index)
	{
		this->set(index, nullptr);
	}
	
	template<typename T>
	void UniformPool<T>::clear()
	{
		std::size_t sz = this->size();
		for(std::size_t i = 0; i < sz; i++)
		{
			this->erase(i);
		}
	}
	
	template<typename T>
	template<typename... Args>
	T& UniformPool<T>::emplace(std::size_t index, Args&&... args)
	{
		topaz_assert(index < this->capacity(), "UniformPool<T>::emplace(index, ...): Emplacing at index", index, " is beyond the capacity of ", this->capacity());
		T* ptr = this->at(index);
		// This element already exists. Invoke its destructor first.
		if(this->is_object(index))
		{
			ptr->~T();
		}
		// Now create the object in-place. No need to use the return as we always launder memory.
		new (ptr) T{std::forward<Args>(args)...};
		// Make sure our mask fits. Anything between our old limit and now aren't going to be objects obviously.
		this->object_mask.resize(index + 1, false);
		// We now have an object here.
		this->object_mask[index] = true;
	}
	
	template<typename T>
	const T& UniformPool<T>::operator[](std::size_t index) const
	{
		topaz_assert(this->is_object(index), "UniformPool<T>::operator[", index, "]: No valid T at the given index!");
		return *this->at(index);
	}
	
	template<typename T>
	T& UniformPool<T>::operator[](std::size_t index)
	{
		topaz_assert(this->is_object(index), "UniformPool<T>::operator[", index, "]: No valid T at the given index!");
		return *this->at(index);
	}
	
	template<typename T>
	template<typename As>
	void UniformPool<T>::debug_print_as() const
	{
#if TOPAZ_DEBUG
		if constexpr(!std::is_same_v<std::make_signed_t<As>, char> && !std::is_same_v<std::make_signed_t<As>, std::byte>)
		{
			// This isn't a char nor byte type. We are about to break strict aliasing.
			// std::cerr << "Warning: About to violate strict aliasing in call to UniformPool<T>::debug_print_as<As>().\n";
			// std::cerr << "Dire Warning: About to purposefully invoke UB.\n";
		}
		std::cerr << "\nMemory Representation: {";
		auto as_begin = reinterpret_cast<As*>(this->begin);
		for(std::size_t i = 0; i < this->size_bytes / sizeof(As); i++)
		{
			// We're dereferencing unitialised memory at some point here unless the container is full.
			// Can probably get away with it on debug but on release it is likely that the extreme danger here is no longer worth the usefulness of doing this.
			std::cerr << " " << *(as_begin + i);
		}
		std::cerr << " }";
		std::cerr << "\nMasked Representation: {";
		for(std::size_t i = 0; i < this->size_bytes / sizeof(T); i++)
		{
			if(this->object_mask.size() > i)
				std::cerr << " " << this->object_mask[i];
			else
				std::cerr << " 0";
		}
		std::cerr << " }\n";
#endif
	}
	
	template<typename T>
	bool UniformPool<T>::is_object(std::size_t index) const
	{
		if(this->object_mask.size() <= index)
			return false;
		return this->object_mask[index];
	}
	
	template<typename T>
	const T* UniformPool<T>::at(std::size_t index) const
	{
		auto tbegin = reinterpret_cast<T*>(this->begin);
		tbegin += index;
		// Launder the memory. This should cost nothing if unneeded.
		// This will be needed for the following case:
		// - T contains a const member. Re-assigning the const member, even through placement-new, is UB on next retrieval without laundering.
		// The use of placement-new to construct Ts means we shouldn't ever need to launder to avoid type-punning UB. We should be fine in that regard.
		return std::launder(tbegin);
	}
	
	template<typename T>
	T* UniformPool<T>::at(std::size_t index)
	{
		auto tbegin = reinterpret_cast<T*>(this->begin);
		tbegin += index;
		// Launder the memory. This should cost nothing if unneeded.
		return std::launder(tbegin);
	}
	
}