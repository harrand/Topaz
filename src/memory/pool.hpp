//
// Created by Harrand on 20/12/2019.
//

#ifndef TOPAZ_POOL_HPP
#define TOPAZ_POOL_HPP
#include "memory/block.hpp"
#include "core/bool.hpp"
#include <vector>

namespace tz::mem
{
	/**
	 * \addtogroup tz_mem Topaz Memory Library (tz::mem)
	 * A collection of low-level abstractions around memory utilities not provided by the C++ standard library. This includes non-owning memory blocks, uniform memory-pools and more.
	 * @{
	 */
	/**
	 * Manages a pre-allocated block of memory, treating it as a contiguous array of Ts.
	 * Useful for managing a memory block representing a persistently-mapped buffer (PMB).
	 * @tparam T - Type of object to store
	 */
	template<typename T>
	class UniformPool
	{
	public:
		using value_type = T;
		/**
		 * Construct a UniformPool between two addresses (inclusive).
		 * Precondition: `begin` and `end` address values both lie within a single range of application-owned memory. If not, or the addresses belong to different allocations, then this will invoke UB without asserting.
		 * @param begin Begin address.
		 * @param end End address.
		 */
		UniformPool(void* begin, void* end);
		/**
		 * Construct a UniformPool to manage an existing memory block.
		 * Precondition: The given memory block is valid. Otherwise, this will invoke UB without asserting.
		 * @param block Memory block to manage.
		 */
		UniformPool(Block block);
		/**
		 * Construct a UniformPool from a beginning address value and a given number of bytes. The begin address should locate a block of available memory large enough for the pool size.
		 * Precondition: `begin` and `reinterpret_cast<char*>(begin) + size_bytes` values lie within a single range of application-owned memory. If not, or the addresses belong to different allocations, then this will invoke UB without asserting.
		 * @param begin Begin address.
		 * @param size_bytes Number of additional bytes after the begin address comprising the pool size.
		 */
		UniformPool(void* begin, std::size_t size_bytes);
		/**
		 * Retrieve the number of confirmed-constructed objects within the pool.
		 * Note: Confirmed-constructed objects are objects which were constructed within the underlying memory block via the pool, and not some external source.
		 * @return Number of confirmed-constructed objects within the pool.
		 */
		std::size_t size() const;
		/**
		 * Retrieve the maximum number of elements which this pool can store.
		 * @return Maximum number of possible elements.
		 */
		std::size_t capacity() const;
		/**
		 * Query as to whether the pool is filled as much as possible with confirmed-constructed objects.
		 * @return True if the pool is full of confirmed-constructed objects. False otherwise.
		 */
		bool full() const;
		/**
		 * Query as to whether the pool contains any confirmed-constructed objects.
		 * @return True if the pool contains *zero* confirmed-constructed objects. False otherwise.
		 */
		bool empty() const;
		/**
		 * Retrieve the capacity of the underlying memory block.
		 * @return Size of the pool's underlying memory block, in bytes.
		 */
		std::size_t capacity_bytes() const;
		/**
		 * Copy a T into the given index within the pool.
		 * Note: If the UniformPool is considered a T[], then the index represents an index into this array.
		 * Precondition: `index < capacity()`. Otherwise, this will assert and invoke UB.
		 * @param index Desired index where the object should be constructed.
		 * @param t Value type.
		 */
		void set(std::size_t index, T t);
		/**
		 * Remove an existing T from the pool at the given index. If no confirmed-constructed T exists at the given index, nothing happens.
		 * Precondition: `index < capacity()`. Otherwise, this will assert and early-out.
		 * @param index Index of the pool to clear any objects out of.
		 */
		void set(std::size_t index, std::nullptr_t);
		/**
		 * Remove an existing T from the pool at the given index. If no confirmed-constructed T exists at the given index, nothing happens.
		 * Precondition: `index < capacity()`. Otherwise, this will assert and early-out.
		 * Note: This is identical to `set(index, nullptr)`.
		 * @param index Index of the pool to clear any objects out of.
		 */
		void erase(std::size_t index);
		/**
		 * Invoke `erase` on all elements of the pool. All confirmed-constructed objects will be removed.
		 */
		void clear();
		/**
		 * Construct a new T into the given index directly within the pool.
		 * Note: If the UniformPool is considered a T[], then the index represents an index into this array.
		 * Precondition: `index < capacity()`. Otherwise, this will assert and invoke UB.
		 * @tparam Args Argument type parameters used to construct the T.
		 * @param index Desired index where the object should be constructed.
		 * @param args Argument type values used to construct the T.
		 * @return Reference to the constructed T. This reference remains valid until the underlying memory pool is freed, or the memory at the index is manipulated externally, or another object is constructed at this index.
		 */
		template<typename... Args>
		T& emplace(std::size_t index, Args&&... args);
		/**
		 * Retrieve the T at the given index.
		 * Note: The T at the given index is required to be valid, but not confirmed-constructed (This is likely to become more strict soon).
		 * Precondition: `index < capacity()`. Otherwise, this will assert and invoke UB.
		 * Precondition: A valid T object exists at the given index. Otherwise, this will invoke UB without asserting.
		 * @param index Desired index where the object is located.
		 * @return Reference to the existing T at the given index.
		 */
		const T& operator[](std::size_t index) const;
		/**
		 * Retrieve the T at the given index.
		 * Note: The T at the given index is required to be valid, but not confirmed-constructed (This is likely to become more strict soon).
		 * Precondition: `index < capacity()`. Otherwise, this will assert and invoke UB.
		 * Precondition: A valid T object exists at the given index. Otherwise, this will invoke UB without asserting.
		 * @param index Desired index where the object is located.
		 * @return Reference to the existing T at the given index.
		 */
		T& operator[](std::size_t index);
		/**
		 * TODO: Document
		 * @tparam As
		 */
		template<typename As = T>
		void debug_print_as() const;

		explicit operator tz::mem::Block() const;
	private:
		/**
		 * TODO: Document
		 * @param index
		 * @return
		 */
		bool is_object(std::size_t index) const;
		/**
		 * TODO: Document
		 * @param index
		 * @return
		 */
		const T* at(std::size_t index) const;
		/**
		 * TODO: Document
		 * @param index
		 * @return
		 */
		T* at(std::size_t index);
		void* begin;
		std::size_t size_bytes;
		std::vector<tz::BoolProxy> object_mask;
	};

	/**
	 * Manages a pre-allocated block of memory, treating it as an existing tuple<Ts...>.
	 * Can be a useful alternative to a UniformPool if the types are known at compile-time.
	 * @tparam Ts - Permutation of parameter types.
	 */
	template<typename... Ts>
	class StaticPool
	{
	public:
		template<std::size_t I>	
		using ValueType = std::tuple_element_t<I, std::tuple<Ts...>>;
		/**
		 * Construct a StaticPool at the given address, using the given values.
		 * Precondition: The begin pointer points to a contiguous block of pre-allocated, unmanaged memory of size >= StaticPool<Ts...>::size(). Otherwise, this will invoke UB without asserting.
		 * @param begin Address containing the beginning of the storage.
		 * @param ts Parameter values comprising the pool.
		 */
		StaticPool(void* begin, Ts&&... ts);
		/**
		 * Construct a StaticPool at the given address. Value-initialises all elements of the pool.
		 * Precondition: The begin pointer points to a contiguous block of pre-allocated, unmanaged memory of size >= StaticPool<Ts...>::size(). Otherwise, this will invoke UB without asserting.
		 * @param begin Address containing the beginning of the storage.
		 */
		StaticPool(void* begin);
		/**
		 * Construct a StaticPool within the given memory block. Initialises all elements using the given values.
		 * Precondition: The block has size() >= StaticPool<Ts...>::size(). Otherwise, this will assert and invoke UB.
		 * Note: If the block has size() > StaticPool<Ts...>, the remainder bytes ahead of the pool storage will be untouched.
		 * @param block Memory block to use as a pool.
		 * @param ts Parameter values comprising the pool.
		 */
		StaticPool(const Block& block, Ts&&... ts);
		/**
		 * Construct a StaticPool within the given memory block. Value-initialises all elements of the pool.
		 * Precondition: The block has size() >= StaticPool<Ts...>::size(). Otherwise, this will assert and invoke UB.
		 * Note: If the block has size() > StaticPool<Ts...>, the remainder bytes ahead of the pool storage will be untouched.
		 * @param block Memory block to use as a pool.
		 */
		StaticPool(const Block& block);
		~StaticPool();

		/**
		 * Retrieve the total number of bytes used in the given storage.
		 * Note: This is *not* guaranteed to be equal to the sizeof sum of Ts.
		 * @return Required size in bytes.
		 */
		static constexpr std::size_t size();

		/**
		 * Retrieve the pool member at the given index of the parameter pack.
		 * Static Precondition: I <= sizeof...(Ts). Otherwise, this will fail to compile.
		 * Example: StaticPool<int, float, char>::get<1>() will retrieve the float.
		 * @tparam I The index of the template parameter pack to retrieve.
		 * @return Immutable reference to the pool member.
		 */
		template<std::size_t I>
		const auto& get() const;

		/**
		 * Assign the pool member at the given index of the parameter pack to a new value.
		 * Static Precondition: I <= sizeof...(Ts). Otherwise, this will fail to compile.
		 * Example: StaticPool<int, float, char>::set<1>(1.0f) is valid usage to assign the float.
		 * @tparam I The index of the template parameter pack to retrieve.
		 * @param Value The value to assign to values[i].
		 */
		template<std::size_t I>
		void set(ValueType<I> value);
	private:
		using Tuple = std::tuple<Ts...>;
		Tuple* values;
	};

	/**
	 * @}
	 */
}

#include "memory/pool.inl"
#endif //TOPAZ_POOL_HPP
