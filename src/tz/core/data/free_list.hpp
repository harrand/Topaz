#ifndef TOPAZ_CORE_DATA_free_list_HPP
#define TOPAZ_CORE_DATA_free_list_HPP
#include "tz/core/data/handle.hpp"
#include "tz/core/types.hpp"
#include "tz/core/debug.hpp"
#include <vector>
#include <iterator>

namespace tz
{
	template<tz::nullable T, tz::container C = std::vector<T>>
	class free_list;

	template<tz::nullable T, tz::container C = std::vector<T>>
	struct free_list_iterator
	{
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		bool operator==(const free_list_iterator<T, C>& lhs) const = default;
		free_list_iterator& operator+=(const difference_type dst)
		{
			for(std::size_t i = 0; i < dst; i++)
			{
				(*this)++;
			}
			return *this;
		}
		free_list_iterator& operator-=(const difference_type dst)
		{
			for(std::size_t i = 0; i < dst; i++)
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
		free_list_iterator operator+(const difference_type dst) const{auto old = *this; return old += dst;}
		free_list_iterator operator-(const difference_type dst) const{auto old = *this; return old -= dst;}

		difference_type operator-(const free_list_iterator& rhs) const {tz::assert(this->l == rhs.l); return this->internal_handle - rhs.internal_handle;}

		T& operator*()
		{
			auto handle = static_cast<free_list<T, C>::handle>(static_cast<tz::hanval>(this->internal_handle));
			return this->l->operator[](handle);
		}

		const T& operator*() const
		{
			auto handle = static_cast<free_list<T, C>::handle>(static_cast<tz::hanval>(this->internal_handle));
			return this->l->operator[](handle);
		}

		free_list<T, C>* l;	
		std::size_t internal_handle;
	};

	template<tz::nullable T, tz::container C>
	class free_list
	{
	public:
		using handle = tz::handle<T>;
		using iterator = free_list_iterator<T, C>;
		friend class free_list_iterator<T, C>;
		free_list() = default;
		free_list(C&& container);
		free_list& operator=(const free_list<T, C>& rhs);
		free_list& operator=(free_list<T, C>&& rhs);
		free_list& operator=(C&& container);

		std::size_t size() const;
		bool empty() const;
		void clear();
		void reserve(std::size_t count);

		iterator begin();
		iterator end();

		handle push_back(T&& t) requires
			requires(C con) {con.push_back(std::forward<T>(t));};

		template<typename... Ts>
		handle emplace_back(Ts&&... ts) requires
			requires(C con) {{con.emplace_back(std::forward<Ts>(ts)...)} -> std::same_as<T&>;};

		void erase(handle h);

		const T& operator[](handle h) const;
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
