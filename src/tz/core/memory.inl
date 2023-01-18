#include <utility>

namespace tz
{
	template<typename T>
	maybe_owned_ptr<T>::maybe_owned_ptr(std::nullptr_t):
	ptr(nullptr)
	{

	}

	template<typename T>
	maybe_owned_ptr<T>::maybe_owned_ptr(T* ptr):
	ptr(ptr)
	{

	}

	template<typename T>
	maybe_owned_ptr<T>::maybe_owned_ptr(std::unique_ptr<T> owned):
	ptr(std::move(owned)){}

	template<typename T>
	template<typename P>
	maybe_owned_ptr<T>::maybe_owned_ptr(maybe_owned_ptr<P>&& move) requires std::derived_from<P, T>:
	maybe_owned_ptr<T>(nullptr)
	{
		if(move.owning())
		{
			*this = static_cast<T*>(move.get());
			move.set_owning(false);
			this->set_owning(true);
		}
		else
		{
			*this = static_cast<T*>(move.get());
		}
	}

	template<typename T>
	maybe_owned_ptr<T>& maybe_owned_ptr<T>::operator=(std::nullptr_t)
	{
		this->ptr = nullptr;
		return *this;
	}

	template<typename T>
	maybe_owned_ptr<T>& maybe_owned_ptr<T>::operator=(T* ptr)
	{
		this->ptr = ptr;
		return *this;
	}

	template<typename T>
	maybe_owned_ptr<T>& maybe_owned_ptr<T>::operator=(std::unique_ptr<T> ptr)
	{
		this->ptr = std::move(ptr);
		return *this;
	}

	template<typename T>
	T* maybe_owned_ptr<T>::get()
	{
		if(this->owning())
		{
			return std::get<std::unique_ptr<T>>(this->ptr).get();
		}
		return std::get<T*>(this->ptr);
	}

	template<typename T>
	const T* maybe_owned_ptr<T>::get() const
	{
		if(this->owning())
		{
			return std::get<std::unique_ptr<T>>(this->ptr).get();
		}
		return std::get<T*>(this->ptr);
	}

	template<typename T>
	bool maybe_owned_ptr<T>::owning() const
	{
		return std::holds_alternative<std::unique_ptr<T>>(this->ptr);
	}

	template<typename T>
	T* maybe_owned_ptr<T>::release()
	{
		if(this->owning())
		{
			T* p = std::get<std::unique_ptr<T>>(this->ptr).release();
			*this = nullptr;
			return p;
		}
		return this->get();
	}

	template<typename T>
	void maybe_owned_ptr<T>::reset()
	{
		*this = nullptr;
	}

	template<typename T>
	void maybe_owned_ptr<T>::set_owning(bool should_own)
	{
		if(should_own == this->owning())
		{
			return;
		}
		if(should_own)
		{
			std::unique_ptr<T> new_owning(this->get());
			*this = std::move(new_owning);
		}
		else
		{
			*this = this->release();
		}
	}

	template<typename T>
	T* maybe_owned_ptr<T>::operator->()
	{
		return this->get();
	}

	template<typename T>
	const T* maybe_owned_ptr<T>::operator->() const
	{
		return this->get();
	}

	template<typename T>
	bool maybe_owned_ptr<T>::operator==(const T* t) const
	{
		return this->get() == t;
	}

	template<typename T, typename... Args>
	maybe_owned_ptr<T> make_owned(Args&&... args)
	{
		return {std::make_unique<T>(std::forward<Args>(args)...)};
	}
}
