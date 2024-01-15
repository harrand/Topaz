#include "tz/core/data/free_list.hpp"

namespace tz
{
	template<tz::nullable T, tz::container C>
	std::size_t free_list<T, C>::size() const
	{
		return this->elements.size() - this->frees.size();
	}

	template<tz::nullable T, tz::container C>
	bool free_list<T, C>::empty() const
	{
		return this->size() == 0;
	}

	template<tz::nullable T, tz::container C>
	void free_list<T, C>::clear()
	{
		this->elements.clear();
		this->frees.clear();
	}

	template<tz::nullable T, tz::container C>
	free_list<T, C>::iterator free_list<T, C>::begin()
	{
		std::size_t begin_id = 0;
		while(begin_id < this->elements.size() && this->is_in_free_list(begin_id))
		{
			begin_id++;
		}
		return {.l = this, .internal_handle = begin_id};
	}

	template<tz::nullable T, tz::container C>
	free_list<T, C>::iterator free_list<T, C>::end()
	{
		std::size_t end_id = this->elements.size();
		while(end_id > 0 && this->is_in_free_list(end_id))
		{
			end_id--;
		}
		return {.l = this, .internal_handle = end_id};
	}

	template<tz::nullable T, tz::container C>
	free_list<T, C>::handle free_list<T, C>::push_back(T&& t)  requires
		requires(C con) {con.push_back(std::forward<T>(t));}
		// implementation note: i dont think the `requires` clause needs to be here. i am only re-defining it coz of a clang bug. https://github.com/llvm/llvm-project/issues/56482 
	{
		handle ret;
		if(this->frees.size())
		{
			ret = this->frees.front();
			this->frees.erase(this->frees.begin());

			// assign to free-list location.
			// previous value must be null.
			T& loc = this->elements[static_cast<std::size_t>(static_cast<tz::hanval>(ret))];
			tz::assert(loc.is_null());
			loc = std::forward<T>(t);
		}
		else
		{
			ret = static_cast<tz::hanval>(this->elements.size());
			// just invoke push back.	
			this->elements.push_back(std::forward<T>(t));
		}
		return ret;
	}

	template<tz::nullable T, tz::container C>
	template<typename... Ts>
	free_list<T, C>::handle free_list<T, C>::emplace_back(Ts&&... ts) requires
		requires(C con) {{con.emplace_back(std::forward<Ts>(ts)...)} -> std::same_as<T&>;}
		// implementation note: i dont think the `requires` clause needs to be here. i am only re-defining it coz of a clang bug. https://github.com/llvm/llvm-project/issues/56482 
	{
		handle ret;
		if(this->frees.size())
		{
			ret = this->frees.front();
			this->frees.erase(this->frees.begin());

			// assign to free-list location.
			// previous value must be null.
			T& loc = this->elements[static_cast<std::size_t>(static_cast<tz::hanval>(ret))];
			tz::assert(loc.is_null());
			loc = T{std::forward<Ts>(ts)...};
		}
		else
		{
			ret = static_cast<tz::hanval>(this->elements.size());
			// just invoke emplace back.	
			this->elements.emplace_back(std::forward<Ts>(ts)...);
		}
		return ret;
	}

	template<tz::nullable T, tz::container C>
	void free_list<T, C>::erase(handle h)
	{
		if(this->is_in_free_list(h))
		{
			return;
		}
		// set the element to empty. add to free-list.
		this->elements[static_cast<std::size_t>(static_cast<tz::hanval>(h))] = T::null();
		this->frees.push_back(h);
	}

	template<tz::nullable T, tz::container C>
	const T& free_list<T, C>::operator[](handle h) const
	{
		return this->elements[static_cast<std::size_t>(static_cast<tz::hanval>(h))];
	}

	template<tz::nullable T, tz::container C>
	T& free_list<T, C>::operator[](handle h)
	{
		return this->elements[static_cast<std::size_t>(static_cast<tz::hanval>(h))];
	}

	template<tz::nullable T, tz::container C>
	bool free_list<T, C>::is_in_free_list(std::size_t internal_id) const
	{
		return this->is_in_free_list(static_cast<tz::hanval>(internal_id));
	}

	template<tz::nullable T, tz::container C>
	bool free_list<T, C>::is_in_free_list(handle h) const
	{
		return std::find(this->frees.begin(), this->frees.end(), h) != this->frees.end();
	}
	
}
