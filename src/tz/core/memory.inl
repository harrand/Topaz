#include <utility>

namespace tz
{
	template<typename T>
	MaybeOwnedPtr<T>::MaybeOwnedPtr(std::nullptr_t):
	ptr(nullptr)
	{

	}

	template<typename T>
	MaybeOwnedPtr<T>::MaybeOwnedPtr(T* ptr):
	ptr(ptr)
	{

	}

	template<typename T>
	MaybeOwnedPtr<T>::MaybeOwnedPtr(std::unique_ptr<T> owned):
	ptr(std::move(owned)){}

	template<typename T>
	template<typename P>
	MaybeOwnedPtr<T>::MaybeOwnedPtr(MaybeOwnedPtr<P>&& move) requires std::derived_from<P, T>:
	MaybeOwnedPtr<T>(nullptr)
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
	MaybeOwnedPtr<T>& MaybeOwnedPtr<T>::operator=(std::nullptr_t)
	{
		this->ptr = nullptr;
		return *this;
	}

	template<typename T>
	MaybeOwnedPtr<T>& MaybeOwnedPtr<T>::operator=(T* ptr)
	{
		this->ptr = ptr;
		return *this;
	}

	template<typename T>
	MaybeOwnedPtr<T>& MaybeOwnedPtr<T>::operator=(std::unique_ptr<T> ptr)
	{
		this->ptr = std::move(ptr);
		return *this;
	}

	template<typename T>
	T* MaybeOwnedPtr<T>::get()
	{
		if(this->owning())
		{
			return std::get<std::unique_ptr<T>>(this->ptr).get();
		}
		return std::get<T*>(this->ptr);
	}

	template<typename T>
	const T* MaybeOwnedPtr<T>::get() const
	{
		if(this->owning())
		{
			return std::get<std::unique_ptr<T>>(this->ptr).get();
		}
		return std::get<T*>(this->ptr);
	}

	template<typename T>
	bool MaybeOwnedPtr<T>::owning() const
	{
		return std::holds_alternative<std::unique_ptr<T>>(this->ptr);
	}

	template<typename T>
	T* MaybeOwnedPtr<T>::release()
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
	void MaybeOwnedPtr<T>::reset()
	{
		*this = nullptr;
	}

	template<typename T>
	void MaybeOwnedPtr<T>::set_owning(bool should_own)
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
	T* MaybeOwnedPtr<T>::operator->()
	{
		return this->get();
	}

	template<typename T>
	const T* MaybeOwnedPtr<T>::operator->() const
	{
		return this->get();
	}

	template<typename T>
	bool MaybeOwnedPtr<T>::operator==(const T* t) const
	{
		return this->get() == t;
	}

	template<typename T, typename... Args>
	MaybeOwnedPtr<T> make_owned(Args&&... args)
	{
		return {std::make_unique<T>(std::forward<Args>(args)...)};
	}
}
