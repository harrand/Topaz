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

template<typename PoolMarking = int>
class PoolMarker
{
public:
    using MarkerType = PoolMarking;
	using MemoryRegion = std::pair<const void*, const void*>;
	void mark(MemoryRegion region, PoolMarking marking);
	void unmark(PoolMarking marking);
	void unmark(MemoryRegion region);
	std::optional<PoolMarking> get_mark(const void* address) const;
	std::vector<PoolMarking> get_marks(const MemoryRegion& region) const;
private:
	void unmark_address(const void* address);

	std::unordered_multimap<PoolMarking, MemoryRegion> marked_regions;
};

/**
 * Manages a pre-allocated but currently unmanaged pool of memory. Although the memory is managed by the class, it does not ever attempt to free the memory; that responsibility remains with the caller.
 * @tparam T - The type of data used in this pool. For pools supporting more than one type, consider a DynamicVariadicMemoryPool.
 */
template<typename T>
class MemoryPool : protected PoolMarker<>
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
     * @tparam ContiguousContainer - Template class for the contiguous container. The container must be contiguous in memory. So std::vector is fine, but std::set is not. SFINAE is used to prevent this constructor from ever being selected over the MemoryPool copy constructor (as MemoryPool<T> substitutes into ContiguousContainer<T>).
     * @param data - Container value to manage memory of
     */
	template<template<typename> typename ContiguousContainer, typename = typename std::enable_if_t<!std::is_same_v<ContiguousContainer<T>, MemoryPool<T>>>>
	MemoryPool(ContiguousContainer<T>& data);
	/**
	 * Assign a MemoryPool to a given ContiguousContainer. This does not change the targeted memory region of the pool, but copies the ContiguousContainer data to the pool.
	 * Note: The ContiguousContainer::size() must not be greater than this element capacity, or an assertion will fail (or invoke UB in Release).
	 * @tparam ContiguousContainer - Template class for the contiguous container. The container must be contiguous in memory. So std::vector is fine, but std::set is not. Unlike the ContiguousContainer constructor, this type can also be a MemoryPool<T>
	 * @param data - Container of data to copy from
	 * @return - Pool to the same memory, after being copied from
	 */
	template<template<typename> typename ContiguousContainer, typename = typename std::enable_if_t<!std::is_same_v<ContiguousContainer<T>, MemoryPool<T>>>>
	MemoryPool<T>& operator=(const ContiguousContainer<T>& data);
	iterator begin();
	const_iterator cbegin() const;
	iterator end();
	const_iterator cend() const;
    void mark(std::size_t index, MarkerType mark);
    void mark_value(const T& element, MarkerType mark);
    void mark_range(iterator begin, iterator end, MarkerType mark);
    void mark_indices(std::size_t begin_index, std::size_t end_index, MarkerType mark);
    void unmark(std::size_t index);
    void unmark_value(const T& element);
    void unmark_range(iterator begin, iterator end);
    void unmark_indices(std::size_t begin_index, std::size_t end_index);
    std::optional<MarkerType> get_mark(std::size_t index) const;
    std::optional<MarkerType> get_value_mark(const T& element) const;
	/**
	 * Get the element at the given index. Range-checking is conditionally compiled via assertions. In Release, there is no range-checking.
	 * @param index - Index of the element to retrieve
	 * @return - Reference to the resultant element
	 */
	T& operator[](std::size_t index);
	/**
	 * Get the immutable element at the given index. Range-checking is conditionally compiled via assertions. In Release, there is no range-checking.
	 * @param index - Index of the element to retrieve
	 * @return - Constant reference to the resultant element
	 */
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
	AutomaticMemoryPool(const MemoryPool<T>& copy);
    /// Deallocates all memory in the pool
	virtual ~AutomaticMemoryPool();
};

template<typename T>
using AMPool = AutomaticMemoryPool<T>;

/**
 * Like a MemoryPool, but the size and composition of the pool is known at compile-time, due to the template parameter pack arguments.
 * This is essentially an std::tuple which assumes that the memory is pre-allocated.
 * @tparam Ts - Type order in the pool, just like an std::tuple
 */
template<typename... Ts>
class StaticVariadicMemoryPool : public MemoryPool<char>
{
public:
	/**
	 * Construct an SVMPool at the given address.
	 * Note: It is assumed that the memory region beginning at this address is preallocated, of size at least totalling the total sizeof the parameter pack. If not, attempting to access this pool will invoke UB.
	 * Note: This will not attempt to default or zero the existing memory. This is achieved by methods. It is your responsibility to track which elements are garbage memory or not.
	 * @param begin_address - Address for this pool to manage
	 */
	StaticVariadicMemoryPool(void* begin_address);
	/**
	 * Construct an SVMPool at the given address. Following that, write the given parameter values to the pool.
	 * Note: It is assumed that the memory region beginning at this address is preallocated, of size at least totalling the total sizeof the parameter pack. If not, attempting to access this pool will invoke UB.
	 * @param begin_address - Address for this pool to manage
	 * @param ts - Parameter arguments to populate the memory pool
	 */
	StaticVariadicMemoryPool(void* begin_address, Ts&&... ts);
	/**
	 * Construct an SVMPool wrapping around an existing byte pool.
	 * Note: It is assumed that the byte capacity of the byte pool is equal to the size totalling the parameter pack. If this is not the case, an assertion will fail (Or invoke UB in Release).
	 * @param fixed_pool - MemoryPool to wrap around
	 */
	StaticVariadicMemoryPool(MemoryPool<char>&& fixed_pool);
	/**
	 * Get the number of elements available in the pool. For an SVMPool, this is always equal to the number of elements in the parameter pack.
	 * @return - Number of elements in the parameter pack
	 */
    virtual std::size_t get_element_capacity() const override;
	/**
	 * Get the number of bytes available in the pool. For an SVMPool, this is always equal to the total size of every element in the parameter pack.
	 * @return - Capacity of the pool, in bytes
	 */
	virtual std::size_t get_byte_capacity() const override;
	/**
	 * Get the given type value in the pool.
	 * Note: This will fail to compile in the case that duplicate types exist in the pool.
	 * Note: To access an SVMPool with duplicate types, use SVMPool<Ts...>::get<std::size_t I>().
	 * @tparam T - Type of the pool element to retrieve
	 * @return - Value corresponding to the given type in the pool
	 */
	template<typename T>
	T& get();
	/**
	 * Get the given type value in the pool.
	 * Note: This will fail to compile in the case that duplicate types exist in the pool.
	 * Note: To access an SVMPool with duplicate types, use SVMPool<Ts...>::get<std::size_t I>().
	 * @tparam T - Type of the pool element to retrieve
	 * @return - Value corresponding to the given type in the pool
	 */
	template<typename T>
	const T& get() const;
	/**
	 * Get the value at the given index in the pool, whatever its type may be.
	 * Note: This will fail to compile if the index is out of range.
	 * Example: SVMPool<float, int, char>::get<0>() will return a value of type float.
	 * @tparam I - Index of the value to retrieve
	 * @return - Value at the given index
	 */
	template<std::size_t I>
	decltype(auto) get();
	/**
	 * Get the value at the given index in the pool, whatever its type may be.
	 * Note: This will fail to compile if the index is out of range.
	 * Example: SVMPool<float, int, char>::get<0>() will return a value of type float.
	 * @tparam I - Index of the value to retrieve
	 * @return - Value at the given index
	 */
	template<std::size_t I>
	decltype(auto) get() const;
	/**
	 * Clear the pool, resetting each element to its default value.
	 * Note: This will fail to compile if at least one type in the parameter pack is not default constructible.
	 */
	virtual void default_all() override;
private:
	using MemoryPool<char>::operator[];
};

template<typename... Ts>
using SVMPool = StaticVariadicMemoryPool<Ts...>;

/**
 * An SVMPool which allocates its own memory. This is an std::tuple for all intents and purposes.
 * @tparam Ts - Type parameters comprising the tuple.
 */
template<typename... Ts>
class AutomaticStaticVariadicMemoryPool : public StaticVariadicMemoryPool<Ts...>
{
public:
	/**
	 * Construct the ASVMPool.
	 * Note: This does not default-construct the parameters. This can be achieved by invoking ASVMPool<Ts...>::default_all() or selecting a different constructor.
	 */
    AutomaticStaticVariadicMemoryPool();
	AutomaticStaticVariadicMemoryPool(const StaticVariadicMemoryPool<Ts...>& copy);
	/**
	 * Construct the ASVMPool. Following that, assign each element to the given parameters.
	 * @param ts - Values of each element to assign.
	 */
    AutomaticStaticVariadicMemoryPool(Ts&&... ts);
    virtual ~AutomaticStaticVariadicMemoryPool();
};

template<typename... Ts>
using ASVMPool = AutomaticStaticVariadicMemoryPool<Ts...>;

/**
 * A very highly useful class making heavy use of RTTI.
 * DVMPools wrap a given memory region for use as an arbitrary, dynamic tuple.
 * You could, for example, push_back a float, then an int, then any class you want, forever!
 * Note: Assertions are used for bounds-checking. In Release, you retain sole responsibility for preventing UB invocation.
 */
class DynamicVariadicMemoryPool : public MemoryPool<char>
{
public:
	/**
	 * Construct a DVMPool at the given address and size.
	 * Note: This does not attempt to edit any of the memory; it will not change. Therefore, you should not be surprised if you see garbage here.
	 * @param begin_address - Beginning address of the pool
	 * @param byte_size - Size of the pool, in bytes
	 */
	DynamicVariadicMemoryPool(void* begin_address, std::size_t byte_size);
	/**
	 * Wrap an existing pool. The inserted size must be equal to the number of elements that have already been managed in the given pool.
	 * Note: If 0 is passed for inserted_size, the pool is assumed to be garbage memory, and push_back invocations will write-over all existing memory. If inserted_size == pool.get_byte_capacity(), then the pool is assumed to already be full.
	 * Note: Even though the parameter is forced to be a pool if any singular type, further push_back invocations need not be the same type; so long as there is enough space remaining to add the values.
	 * @tparam T - POD type of the existing pool
	 * @param pool - Existing pool to wrap around
	 * @param inserted_size - Number of elements in the pool that are not garbage values (in-order)
	 */
    template<typename T>
    DynamicVariadicMemoryPool(MemoryPool<T>&& pool, std::size_t inserted_size);
	/**
	 * Attempt to insert a new value into the DVMPool.
	 * Note: Assertions are used for bounds checking. If you attempt to insert a value which does not fit, an assertion will fail (Or in Release, UB will be invoked).
	 * Note: This does not insert a value at the very end of the pool. It inserts a value at the next memory location after the begin-address which is not known to be used. This essentially will only ever overwrite garbage memory.
	 * @tparam T - Value type to insert at the back of the managed pool
	 * @param value - Value to insert at the back of the managed pool
	 */
	template<typename T>
	void push_back(T value);
	/**
	 * Get the number of elements in this DVMPool.
	 * Example: For a DVMPool containing a float and an int, with a capacity of 1024 bytes, this method will return 2.
	 * @return - Number of elements comprising the DVMPool
	 */
	std::size_t get_size() const;
	/**
	 * Get the total number of bytes used in the pool. Not to be confused with DVMPool::get_byte_capacity().
	 * Example: For a DVMPool containing a float and an int, with a capacity of 1024 bytes, this method will return sizeof(float) + sizeof(int).
	 * @return - Bytes used in the managed section of the pool
	 */
	std::size_t get_byte_usage() const;
	/**
	 * Use RTTI to retrieve the Nth type in this DVMPool.
	 * Note: This uses formal range-checking, and will throw an std::out_of_range if the index is out of range.
	 * Example: For a DVMPool containing a float and an int, with a capacity of 1024 bytes and an index of 0, this method will return typeid(float).
	 * @param index - Index of the type to retrieve
	 * @return - Standard library type_index corresponding to the type at the given index
	 */
	const std::type_index& get_type_at_index(std::size_t index) const;
	/**
	 * Get the value at the given index.
	 * Note: typeid(T) must be equal to get_type_at_index(index). An assertion ensures this. In Release, no such assurance exists, and will invoke UB if you give the wrong type.
	 * @tparam T - Type of the element to retrieve
	 * @param index - Index to retrieve the value from
	 * @return - Reference to the value at the given index
	 */
	template<typename T>
	T& at(std::size_t index);
	/**
	 * Get the value at the given index.
	 * Note: typeid(T) must be equal to get_type_at_index(index). An assertion ensures this. In Release, no such assurance exists, and will invoke UB if you give the wrong type.
	 * @tparam T - Type of the element to retrieve
	 * @param index - Index to retrieve the value from
	 * @return - Reference to the value at the given index
	 */
	template<typename T>
	const T& at(std::size_t index) const;
	/**
	 * Zero all of the memory in the pool. For a DVMPool, this will also clear the pool of all type management.
	 * This means that the whole pool will once again be treated as garbage, and push_back will insert from the begin-address.
	 */
	virtual void zero_all() override;
    friend class AutomaticDynamicVariadicMemoryPool;
protected:
	using MemoryPool<char>::get_element_capacity;
	using MemoryPool<char>::default_all;
	using MemoryPool<char>::operator[];
	void* get_current_offset() const;
	void* get_offset_to_index(std::size_t index) const;

	/// This keeps track of which types comprise the DVMPool in which order.
	std::vector<std::type_index> type_format;
	/// This stores the size of each type used in the DVMPool.
	std::unordered_map<std::type_index, std::size_t> type_size_map;
};

using DVMPool = DynamicVariadicMemoryPool;

/**
 * This is a DVMPool which allocates its own memory.
 * This is currently unused, but I suspect will be used if Lua integration ever happens.
 * If you just want to store a tuple of known types, consider either using ASVMPool or simply std::tuple.
 * This class should be preferred when you do not know the types at compile-time.
 */
class AutomaticDynamicVariadicMemoryPool : public DynamicVariadicMemoryPool
{
public:
	/**
	 * Construct the ADVMPool with a given number of bytes.
	 * @param byte_size - Number of bytes comprising the pool
	 */
	AutomaticDynamicVariadicMemoryPool(std::size_t byte_size);
    AutomaticDynamicVariadicMemoryPool(const DynamicVariadicMemoryPool& copy);
	virtual ~AutomaticDynamicVariadicMemoryPool();
};

using ADVMPool = AutomaticDynamicVariadicMemoryPool;

namespace tz::utility::memory
{
	/**
	 * Partition an existing MemoryPool into an SVMPool of known components and the remainder of the pool.
	 * @tparam T - Type of the MemoryPool
	 * @tparam Ts - Types comprising the daughter SVMPool
	 * @param initial_pool - Initial pool to partition
	 * @return - Pair, where first element is a daughter SVMPool wrapping the first of the initial pool, and a second MemoryPool wrapping the remainder of the initial pool
	 */
	template<typename T, typename... Ts>
	std::pair<SVMPool<Ts...>, MemoryPool<T>> partition(MemoryPool<T> initial_pool);
}

#include "utility/memory.inl"
#endif //TOPAZ_MEMORY_HPP