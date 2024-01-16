#ifndef TOPAZ_CORE_DATA_free_list_HPP
#define TOPAZ_CORE_DATA_free_list_HPP
#include "tz/core/data/handle.hpp"
#include "tz/core/types.hpp"
#include "tz/core/debug.hpp"
#include <vector>
#include <iterator>
#include <type_traits>

namespace tz
{
	template<tz::nullable T, tz::random_access_container C = std::vector<T>>
	class free_list;

	template<tz::nullable T, tz::random_access_container C = std::vector<T>, typename F = free_list<T, C>>
	struct free_list_iterator
	{
		using iterator_category = std::random_access_iterator_tag;
		using value_type = std::conditional_t<std::is_const_v<F>, const T, T>;
		using difference_type = std::ptrdiff_t;
		using pointer = std::conditional_t<std::is_const_v<F>, const T*, T*>;
		using reference = std::conditional_t<std::is_const_v<F>, const T&, T&>;

		auto operator<=>(const free_list_iterator<T, C, F>& lhs) const = default;
		free_list_iterator& operator+=(const std::integral auto dst)
		{
			for(difference_type i = 0; i < dst; i++)
			{
				(*this)++;
			}
			return *this;
		}
		free_list_iterator& operator-=(const std::integral auto dst)
		{
			for(difference_type i = 0; i < dst; i++)
			{
				(*this)--;
			}
			return *this;
		}
		free_list_iterator& operator++()
		{
			while(this->internal_handle < this->l->elements.size() && this->l->is_in_free_list(++this->internal_handle));
			return *this;
		}
		free_list_iterator& operator--()
		{
			tz::assert(this->internal_handle > 0);
			while(this->internal_handle > 0 && this->l->is_in_free_list(--this->internal_handle));
			return *this;
		}
		free_list_iterator operator++(int){auto tmp = *this; --tmp; return tmp;}
		free_list_iterator operator--(int){auto tmp = *this; --tmp; return tmp;}
		free_list_iterator operator+(const std::integral auto dst) const{auto old = *this; return old += dst;}
		free_list_iterator operator-(const std::integral auto dst) const{auto old = *this; return old -= dst;}

		difference_type operator-(const free_list_iterator& rhs) const {tz::assert(this->l == rhs.l); return this->internal_handle - rhs.internal_handle;}

		T& operator[](difference_type i) requires(!std::is_const_v<F>)
		{
			auto iter_cpy = (*this) + i;
			return *iter_cpy;
		}

		std::conditional_t<std::is_const_v<F>, const T&, T&> operator[](difference_type i) const
		{
			auto iter_cpy = (*this) + i;
			return *iter_cpy;
		}

		T& operator*() requires(!std::is_const_v<F>)
		{
			auto handle = static_cast<free_list<T, C>::handle>(static_cast<tz::hanval>(this->internal_handle));
			return this->l->operator[](handle);
		}

		std::conditional_t<std::is_const_v<F>, const T&, T&> operator*() const
		{
			auto handle = static_cast<F::handle>(static_cast<tz::hanval>(this->internal_handle));
			return this->l->operator[](handle);
		}

		// note: a mutable iterator needs to be implicitly convertible to a const iterator.
		// however, a const iterator shouldn't be implicitly convertible to a const-const iterator (hence the concept constraint.)
		operator free_list_iterator<T, C, const F>() const requires(!std::is_const_v<F>)
		{
			return {.l = this->l, .internal_handle = this->internal_handle};
		}

		F* l;	
		std::size_t internal_handle;
	};

	// support for int + free_list_iterator
	// as operator+ needs to be commutative to be a proper random_access_iterator.

	template<tz::nullable T, tz::random_access_container C = std::vector<T>, typename F>
	free_list_iterator<T, C, F> operator+(std::integral auto lhs, const free_list_iterator<T, C, F>& iter)
	{
		return iter + lhs;
	}

	template<tz::nullable T, tz::random_access_container C = std::vector<T>, typename F>
	free_list_iterator<T, C, F> operator-(std::integral auto lhs, const free_list_iterator<T, C, F>& iter)
	{
		return iter - lhs;
	}


	/**
	 * @ingroup tz_core_data
	 * Bespoke free-list implementation. Container adaptor that deals with stable, opaque handles to nullable elements. Erased elements are represented by the `null` value. Erased elements are re-cycled in subsequent new elements.
	 * - No pointer stability.
	 * - Random access.
	 * - Index stability (via opaque handles).
	 * - No agency over orderedness.
	 * - push_back and emplace_back may re-use old slots instead of actually adding to the end of the list.
	 */ 
	template<tz::nullable T, tz::random_access_container C>
	class free_list
	{
	public:
		using handle = tz::handle<T>;
		using iterator = free_list_iterator<T, C, free_list<T, C>>;
		using const_iterator = free_list_iterator<T, C, const free_list<T, C>>;
		friend struct free_list_iterator<T, C, free_list<T, C>>;
		friend struct free_list_iterator<T, C, const free_list<T, C>>;
		/// Create an empty free list.
		free_list() = default;
		free_list(const free_list<T, C>& cpy) = default;
		free_list(free_list<T, C>&& move) = default;
		free_list(C&& container);
		free_list& operator=(const free_list<T, C>& rhs);
		free_list& operator=(free_list<T, C>&& rhs);
		free_list& operator=(C&& container);

		/// Retrieve the number of elements.
		std::size_t size() const;
		/// Query as to whether the list is empty. Equivalent to `size() == 0`.
		bool empty() const;
		/// Remove all elements, and clear out the underlying free-list. Next handle returned is guaranteed to be 0. Invalidates all references and iterators.
		void clear();
		/// Invoke `reserve(count)` on the underlying container. Does not invalidate iterators. Only invalidates references if the underlying container's `reserve` method also does so.
		void reserve(std::size_t count);

		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;

		/**
		 * Add a copy of a new element to the list.
		 * - Invalidates iterators and references.
		 * @note `push_back` is a poor name for this function. If an element has previously been erased, its location in the list may be re-used for this new element, instead of adding the element at the end.
		 * O(x) where `x` is the worst-case time complexity of the underlying container's `emplace_back` method.
		 * θ(1)
		 */ 
		handle push_back(const T& t) requires
			requires(C con, T t) {con.push_back(t);};

		/**
		 * Move a new element into the list.
		 * - Invalidates iterators and references.
		 * @note `push_back` is a poor name for this function. If an element has previously been erased, its location in the list may be re-used for this new element, instead of adding the element at the end.
		 * O(x) where `x` is the worst-case time complexity of the underlying container's `emplace_back` method.
		 * θ(1)
		 */ 
		handle push_back(T&& t) requires
			requires(C con, T&& t) {con.push_back(std::move(t));};

		/**
		 * Construct a new element in-place within the list.
		 * - Invalidates iterators and references.
		 * @note `emplace_back` is a poor name for this function. If an element has previously been erased, its location in the list may be re-used for this new element, instead of adding the element at the end.
		 * O(x) where `x` is the worst-case time complexity of the underlying container's `emplace_back` method.
		 * θ(1)
		 */ 
		template<typename... Ts>
		handle emplace_back(Ts&&... ts) requires
			requires(C con) {{con.emplace_back(std::forward<Ts>(ts)...)} -> std::same_as<T&>;};

		/**
		 * Erase an existing element from the list using its handle.
		 * - Invalidates iterators, but does not invalidate references.
		 * O(m), where `m` is the number of previously-freed elements that have not yet been recycled. Unaffected by `size()` of the free list.
		 * θ(1)
		 */ 
		void erase(handle h);

		/**
		 * Retrieve the first non-null element within the list.
		 * O(m), where `m` is the number of previously-freed elements that have not yet been recycled.
		 * θ(1)
		 */
		const T& front() const;
		/**
		 * Retrieve the first non-null element within the list.
		 * O(m), where `m` is the number of previously-freed elements that have not yet been recycled.
		 * θ(1)
		 */
		T& front();
		/**
		 * Retrieve the last non-null element within the list.
		 * O(m), where `m` is the number of previously-freed elements that have not yet been recycled.
		 * θ(1)
		 */
		const T& back() const;
		/**
		 * Retrieve the last non-null element within the list.
		 * O(m), where `m` is the number of previously-freed elements that have not yet been recycled.
		 * θ(1)
		 */
		T& back();

		/**
		 * Retrieve the element value corresponding to the provided handle.
		 * O(x) where `x` is the worst-case time complexity of the underlying container's `operator[]` method.
		 * θ(x) where `x` is the worst-case time complexity of the underlying container's `operator[]` method.
		 */
		const T& operator[](handle h) const;
		/**
		 * Retrieve the element value corresponding to the provided handle.
		 * O(x) where `x` is the worst-case time complexity of the underlying container's `operator[]` method.
		 * θ(x) where `x` is the worst-case time complexity of the underlying container's `operator[]` method.
		 */
		T& operator[](handle h);
	private:
		bool is_in_free_list(std::size_t internal_id) const;
		bool is_in_free_list(handle h) const;

		C elements = {};
		std::vector<handle> frees = {};
	};
}

#include "tz/core/data/free_list.inl"
#endif // TOPAZ_CORE_DATA_free_list_HPP
