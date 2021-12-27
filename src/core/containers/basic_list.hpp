#ifndef TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
#define TOPAZ_GL_VK_COMMON_BASIC_LIST_HPP
#include "core/assert.hpp"
#include <initializer_list>
#include <vector>
#include <algorithm>
#include <span>

namespace tz
{
	/**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
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

		BasicList(std::initializer_list<T> elements):
		elements(elements){}

		BasicList() = default;

		operator std::span<const T>() const
		{
			return static_cast<std::span<const T>>(this->elements);
		}

		T& front()
		{
			return this->elements.front();
		}

		const T& front() const
		{
			return this->elements.front();
		}

		T& back()
		{
			return this->elements.back();
		}

		const T& back() const
		{
			return this->elements.back();
		}

		void add(const T& element)
		{
			this->elements.push_back(element);
		}

		void add(T&& element)
		{
			this->elements.push_back(std::forward<T>(element));
		}

		template<typename... Args>
		T& emplace(Args&&... args)
		{
			return this->elements.emplace_back(std::forward<Args>(args)...);
		}

		void append(const BasicList<T>& other)
		{
			this->elements.insert(this->elements.end(), other.elements.begin(), other.elements.end());
		}

		template<typename InputIt>
		Iterator append_range(InputIt first, InputIt last)
		{
			return this->elements.insert(this->elements.end(), first, last);
		}


		bool contains(T element) const
		{
			return std::find(this->elements.begin(), this->elements.end(), element) != this->elements.end();
		}

		Iterator begin()
		{
			return this->elements.begin();
		}

		Iterator end()
		{
			return this->elements.end();
		}

		ConstIterator begin() const
		{
			return this->elements.begin();
		}

		ConstIterator end() const
		{
			return this->elements.end();
		}

		auto length() const
		{
			return this->elements.size();
		}

		bool empty() const
		{
			return this->elements.empty();
		}

		const T* data() const
		{
			return this->elements.data();
		}

		T* data()
		{
			return this->elements.data();
		}

		const T& operator[](std::size_t index) const
		{
			tz_assert(this->length() > index, "tz::BasicList<T>::operator[%zu]: Out of range (length = %zu)", index, this->length());
			return this->elements[index];
		}

		T& operator[](std::size_t index)
		{
			tz_assert(this->length() > index, "tz::BasicList<T>::operator[%zu]: Out of range (length = %zu)", index, this->length());
			return this->elements[index];
		}

		auto operator<=>(const tz::BasicList<T, Allocator>& rhs) const = default;

		Iterator erase(Iterator position)
		{
			return this->elements.erase(position);
		}

		Iterator erase(Iterator first, Iterator last)
		{
			return this->elements.erase(first, last);
		}

		void clear()
		{
			this->elements.clear();
		}

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
