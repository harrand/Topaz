#ifndef TOPAZ_GL_VK_COMMON_POLYMORPHIC_LIST_HPP
#define TOPAZ_GL_VK_COMMON_POLYMORPHIC_LIST_HPP
#include "tz/core/data/basic_list.hpp"
#include <memory>

namespace tz
{
	template<typename T>
	class InterfaceIterator;

	/**
	 * @ingroup tz_core_data
	 * @tparam T Common base type.
	 * List of elements, where all elements inherit from a common type but may be different types within themselves.
	 */
	template<typename T, typename Allocator = std::allocator<std::unique_ptr<T>>>
	class PolymorphicList
	{
	public:
		using Pointer = T*;
		using Reference = T&;
		using Iterator = InterfaceIterator<T>;

		/**
		 * Create an empty list.
		 */
		PolymorphicList() = default;
		/**
		 * Add a new element to the back of the list.
		 * @tparam Derived Type of the element to construct. It must be a subtype of `T`.
		 * @tparam Args Types used to construct the new Derived element.
		 * @param args Argument values for construction.
		 */
		template<typename Derived, typename... Args>
		Reference emplace(Args&&... args);
		/**
		 * Retrieve the begin interator.
		 */
		Iterator begin();
		/**
		 * Retrieve the end interator.
		 */
		Iterator end();
		/**
		 * Retrieve the begin interator.
		 */
		Iterator begin() const;
		/**
		 * Retrieve the begin interator.
		 */
		Iterator end() const;
		/**
		 * Retrieve the number of elements.
		 * @return number of elements within the list.
		 */
		std::size_t size() const;
		/**
		 * Remove all elements from the list.
		 */
		void clear();
		/**
		 * Retrieve a base-class reference to the element at the given index. Read-only.
		 * @pre idx < this->size(), otherwise the behaviour is undefined.
		 */
		const T& operator[](std::size_t idx) const;
		/**
		 * Retrieve a base-class reference to the element at the given index.
		 * @pre idx < this->size(), otherwise the behaviour is undefined.
		 */
		T& operator[](std::size_t idx);
	private:
		using SmartPointer = std::unique_ptr<T>;
		using SmartPointerConst = std::unique_ptr<const T>;
		BasicList<SmartPointer, Allocator> elements;
	};

	template<typename T>
	class InterfaceIterator
	{
	public:
		InterfaceIterator(const std::unique_ptr<T>* loc);
		
		T& operator*();
		const T& operator*() const;
		T* operator->();
		const T* operator->() const;
		InterfaceIterator<T>& operator++();
		InterfaceIterator<T>& operator++(int);
		bool operator==(const InterfaceIterator<T>& rhs) const;
	private:
		const std::unique_ptr<T>* loc;
	};
}

#include "tz/core/data/polymorphic_list.inl"
#endif // TOPAZ_GL_VK_COMMON_POLYMORPHIC_LIST_HPP
