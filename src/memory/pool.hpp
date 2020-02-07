//
// Created by Harrand on 20/12/2019.
//

#ifndef TOPAZ_POOL_HPP
#define TOPAZ_POOL_HPP
#include "memory/block.hpp"
#include <vector>

namespace tz::mem
{
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
}

#include "memory/pool.inl"
#endif //TOPAZ_POOL_HPP
