//
// Created by Harrand on 20/12/2019.
//

#ifndef TOPAZ_POOL_HPP
#define TOPAZ_POOL_HPP
#include "memory/block.hpp"
#include <vector>

namespace tz::mem
{
	/**
     * \addtogroup tz_mem Topaz Memory Library (tz::mem)
     * A collection of low-level abstractions around memory utilities not provided by the C++ standard library. This includes non-owning memory blocks, uniform memory-pools and more.
     * @{
     */

	namespace detail
	{
		class BoolProxy
		{
		public:
			BoolProxy(bool b): b(b){}
			operator bool() const{return this->b;}
		private:
			bool b;
		};
	}
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
		 * TODO: Document
		 * @param begin
		 * @param end
		 */
		UniformPool(void* begin, void* end);
		/**
		 * TODO: Document
		 * @param block
		 */
		UniformPool(Block block);
		/**
		 * TODO: Document
		 * @param begin
		 * @param size_bytes
		 */
		UniformPool(void* begin, std::size_t size_bytes);
		/**
		 * TODO: Document
		 * @return
		 */
		std::size_t size() const;
		/**
		 * TODO: Document
		 * @return
		 */
		std::size_t capacity() const;
		/**
		 * TODO: Document
		 * @return
		 */
		bool full() const;
		/**
		 * TODO: Document
		 * @return
		 */
		bool empty() const;
		/**
		 * TODO: Document
		 * @return
		 */
		std::size_t capacity_bytes() const;
		/**
		 * TODO: Document
		 * @param index
		 * @param t
		 */
		void set(std::size_t index, T t);
		/**
		 * TODO: Document
		 * @param index
		 */
		void set(std::size_t index, std::nullptr_t);
		/**
		 * TODO: Document
		 * @param index
		 */
		void erase(std::size_t index);
		/**
		 * TODO: Document
		 */
		void clear();
		/**
		 * TODO: Document
		 * @tparam Args
		 * @param index
		 * @param args
		 * @return
		 */
		template<typename... Args>
		T& emplace(std::size_t index, Args&&... args);
		/**
		 * TODO: Document
		 * @param index
		 * @return
		 */
		const T& operator[](std::size_t index) const;
		/**
		 * TODO: Document
		 * @param index
		 * @return
		 */
		T& operator[](std::size_t index);
		/**
		 * TODO: Document
		 * @tparam As
		 */
		template<typename As = T>
		void debug_print_as() const;
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
		std::vector<detail::BoolProxy> object_mask;
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
