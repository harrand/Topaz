#ifndef TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
#define TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
#include "tz/core/debug.hpp"
#include <initializer_list>
#include <vector>
#include <algorithm>
#include <span>

namespace tz
{
	/**
	 * @ingroup tz_core_data
	 * Custom list, feature subset of `std::vector`.
	 *
	 * Should be used when a list of contiguous elements is required to be exposed via an API. We never want to expose standard library containers (aside from span).
	 * @tparam T Element type.
	 * @tparam Allocator Allocator type.
	 */
	template<typename T, typename Allocator = std::allocator<T>>
	class BasicList
	{
	private:
		using UnderlyingList = std::vector<T, Allocator>;
		UnderlyingList elements;
	public:
		using Iterator = typename UnderlyingList::iterator;
		using ConstIterator = typename UnderlyingList::const_iterator;

		/**
		 * Construct a BasicList using existing elements via copy.
		 */
		BasicList(std::initializer_list<T> elements):
		elements(elements){}
		/**
		 * Construct an empty BasicList.
		 */
		BasicList() = default;

		/**
		 * Retrieve a span over the entire list.
		 * @return Read-only span over the list.
		 */
		operator std::span<const T>() const
		{
			return static_cast<std::span<const T>>(this->elements);
		}

		/**
		 * Retrieve the first elements of the list.
		 * @pre List must not be empty, otherwise the behaviour is undefined.
		 */
		T& front()
		{
			return this->elements.front();
		}

		/**
		 * Retrieve the first elements of the list. Read-only.
		 * @pre List must not be empty, otherwise the behaviour is undefined.
		 */
		const T& front() const
		{
			return this->elements.front();
		}


		/**
		 * Retrieve the last element of the list.
		 * @pre List must not be empty, otherwise the behaviour is undefined.
		 */
		T& back()
		{
			return this->elements.back();
		}

		/**
		 * Retrieve the last element of the list. Read-only.
		 * @pre List must not be empty, otherwise the behaviour is undefined.
		 */
		const T& back() const
		{
			return this->elements.back();
		}

		/**
		 * Add a copy of the provided value to the back of the list.
		 */
		void add(const T& element)
		{
			this->elements.push_back(element);
		}

		/**
		 * Add the provided value to the back of the list.
		 */
		void add(T&& element)
		{
			this->elements.push_back(std::forward<T>(element));
		}

		/**
		 * Create a new value in-place at the end of the list.
		 * @tparam Args Represents the types used to invoke the element class constructor.
		 * @return Reference to the constructed value.
		 */
		template<typename... Args>
		T& emplace(Args&&... args)
		{
			return this->elements.emplace_back(std::forward<Args>(args)...);
		}

		/**
		 * Copy the elements of another list to the end of this list.
		 * @param other Second list whose elements should be appended to the end of this list.
		 */
		void append(const BasicList<T>& other)
		{
			this->elements.insert(this->elements.end(), other.elements.begin(), other.elements.end());
		}

		/**
		 * Copy the range of elements to the end of this list.
		 * @return Iterator pointing to the first element inserted, or the end of the list if the range was empty.
		 */
		template<typename InputIt>
		Iterator append_range(InputIt first, InputIt last)
		{
			return this->elements.insert(this->elements.end(), first, last);
		}

		/**
		 * Query as to whether the list contains a given element.
		 * @return True if the element is within the list (equality comparison), otherwise false.
		 */
		bool contains(T element) const
		{
			return std::find(this->elements.begin(), this->elements.end(), element) != this->elements.end();
		}

		/**
		 * Retrieve an iterator to the beginning of the list.
		 */
		Iterator begin()
		{
			return this->elements.begin();
		}

		/**
		 * Retrieve an iterator to the end of the list.
		 */
		Iterator end()
		{
			return this->elements.end();
		}

		/**
		 * Retrieve an iterator to the beginning of the list.
		 */
		ConstIterator begin() const
		{
			return this->elements.begin();
		}

		/**
		 * Retrieve an iterator to the end of the list.
		 */
		ConstIterator end() const
		{
			return this->elements.end();
		}

		/**
		 * Retrive the number of elements in the list.
		 */
		auto length() const
		{
			return this->elements.size();
		}

		/**
		 * Query as to whether the list is empty or not.
		 * @return True if `length() == 0`, otherwise false.
		 */
		bool empty() const
		{
			return this->elements.empty();
		}

		/**
		 * Retrieve a pointer to the first element of the list. Read-only.
		 */
		const T* data() const
		{
			return this->elements.data();
		}

		/**
		 * Retrieve a pointer to the first element of the list.
		 */
		T* data()
		{
			return this->elements.data();
		}

		/**
		 * Retrieve the element at a given index. Read-only.
		 * @pre `index < this->length()`, otherwise the behaviour is undefined.
		 * @param index Index whose element should be retrieved. Must be in-range (see preconditions).
		 * @return Reference to the element at the provided index.
		 */
		const T& operator[](std::size_t index) const
		{
			tz::assert(this->length() > index, "tz::BasicList<T>::operator[%zu]: Out of range (length = %zu)", index, this->length());
			return this->elements[index];
		}

		/**
		 * Retrieve the element at a given index.
		 * @pre `index < this->length()`, otherwise the behaviour is undefined.
		 * @param index Index whose element should be retrieved. Must be in-range (see preconditions).
		 * @return Reference to the element at the provided index.
		 */
		T& operator[](std::size_t index)
		{
			tz::assert(this->length() > index, "tz::BasicList<T>::operator[%zu]: Out of range (length = %zu)", index, this->length());
			return this->elements[index];
		}

		auto operator<=>(const tz::BasicList<T, Allocator>& rhs) const = default;

		/**
		 * Erase the element at the given position.
		 * @param position Iterator locating the element which should be erased. Invalidates iterators and references.
		 * @return Iterator following the erased element.
		 */
		Iterator erase(Iterator position)
		{
			return this->elements.erase(position);
		}

		/**
		 * Erase the range of elements bounded within the provided iterator range.
		 * @param first Iterator indicating the beginning of the range to delete.
		 * @param last Iterator indicating the end of the range fo delete.
		 * @return Iterator following the last erased element.
		 */
		Iterator erase(Iterator first, Iterator last)
		{
			return this->elements.erase(first, last);
		}

		/**
		 * Erase the entire list, making it empty again and destroying everything inside.
		 */
		void clear()
		{
			this->elements.clear();
		}

		/**
		 * Resize the list, matches the behaviour of `std::vector<T>::erase(num_elements)`.
		 */
		void resize(std::size_t num_elements)
		{
			this->elements.resize(num_elements);
		}
	};

	/**
	 * @}
	 */
}

#endif // TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
