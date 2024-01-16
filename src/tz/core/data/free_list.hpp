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
	template<tz::nullable T, tz::container C = std::vector<T>>
	class free_list;

	template<tz::nullable T, tz::container C = std::vector<T>, typename F = free_list<T, C>>
	struct free_list_iterator
	{
		using iterator_category = std::random_access_iterator_tag;
		using value_type = std::conditional_t<std::is_const_v<F>, const T, T>;
		using difference_type = std::ptrdiff_t;
		using pointer = std::conditional_t<std::is_const_v<F>, const T*, T*>;
		using reference = std::conditional_t<std::is_const_v<F>, const T&, T&>;

		auto operator<=>(const free_list_iterator<T, C, F>& lhs) const = default;
		free_list_iterator& operator+=(const difference_type dst)
		{
			for(difference_type i = 0; i < dst; i++)
			{
				(*this)++;
			}
			return *this;
		}
		free_list_iterator& operator-=(const difference_type dst)
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
		free_list_iterator operator+(const difference_type dst) const{auto old = *this; return old += dst;}
		free_list_iterator operator-(const difference_type dst) const{auto old = *this; return old -= dst;}

		difference_type operator-(const free_list_iterator& rhs) const {tz::assert(this->l == rhs.l); return this->internal_handle - rhs.internal_handle;}

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

		operator free_list_iterator<T, C, const F>() const
		{
			return {.l = this->l, .internal_handle = this->internal_handle};
		}

		F* l;	
		std::size_t internal_handle;
	};

	template<tz::nullable T, tz::container C>
	class free_list
	{
	public:
		using handle = tz::handle<T>;
		using iterator = free_list_iterator<T, C, free_list<T, C>>;
		using const_iterator = free_list_iterator<T, C, const free_list<T, C>>;
		friend struct free_list_iterator<T, C, free_list<T, C>>;
		friend struct free_list_iterator<T, C, const free_list<T, C>>;
		free_list() = default;
		free_list(const free_list<T, C>& cpy) = default;
		free_list(free_list<T, C>&& move) = default;
		free_list(C&& container);
		free_list& operator=(const free_list<T, C>& rhs);
		free_list& operator=(free_list<T, C>&& rhs);
		free_list& operator=(C&& container);

		std::size_t size() const;
		bool empty() const;
		void clear();
		void reserve(std::size_t count);

		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;

		handle push_back(const T& t) requires
			requires(C con, T t) {con.push_back(t);};

		handle push_back(T&& t) requires
			requires(C con, T&& t) {con.push_back(std::move(t));};

		template<typename... Ts>
		handle emplace_back(Ts&&... ts) requires
			requires(C con) {{con.emplace_back(std::forward<Ts>(ts)...)} -> std::same_as<T&>;};

		void erase(handle h);

		const T& front() const;
		T& front();
		const T& back() const;
		T& back();

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
