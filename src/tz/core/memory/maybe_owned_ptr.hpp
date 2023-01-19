#ifndef TOPAZ_CORE_MEMORY_MAYBE_OWNED_PTR_HPP
#define TOPAZ_CORE_MEMORY_MAYBE_OWNED_PTR_HPP
#include "tz/core/memory/memblk.hpp"

namespace tz
{
	template<typename T>
	class maybe_owned_ptr
	{
	public:
		maybe_owned_ptr(std::nullptr_t);
		maybe_owned_ptr(T* ptr);
		maybe_owned_ptr(std::unique_ptr<T> owned);
		template<typename P>
		maybe_owned_ptr(maybe_owned_ptr<P>&& move) requires std::derived_from<P, T>;

		maybe_owned_ptr(const maybe_owned_ptr<T>& copy) = default;
		maybe_owned_ptr(maybe_owned_ptr<T>&& move) = default;
		maybe_owned_ptr& operator=(const maybe_owned_ptr<T>& rhs) = default;
		maybe_owned_ptr& operator=(maybe_owned_ptr<T>&& rhs) = default;
		maybe_owned_ptr& operator=(std::nullptr_t);
		maybe_owned_ptr& operator=(T* ptr);
		maybe_owned_ptr& operator=(std::unique_ptr<T> ptr);

		T* get();
		const T* get() const;
		bool owning() const;
		void reset();
		T* release();
		void set_owning(bool should_own);

		template<typename P>
		P* as() requires std::derived_from<P, T>
		{
			return static_cast<P*>(this->get());
		}

		template<typename P>
		const P* as() const requires std::derived_from<P, T>
		{
			return static_cast<const P*>(this->get());
		}
		
		T* operator->();
		const T* operator->() const;
		bool operator==(const T* t) const;
	private:
		using variant = std::variant<T*, std::unique_ptr<T>>;
		variant ptr;
	};

	template<typename T, typename... Args>
	maybe_owned_ptr<T> make_owned(Args&&... args);
}

#include "tz/core/memory/maybe_owned_ptr.inl"
#endif // TOPAZ_CORE_MEMORY_MAYBE_OWNED_PTR_HPP
