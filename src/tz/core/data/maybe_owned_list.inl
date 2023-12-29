#include "tz/core/debug.hpp"

namespace tz
{
	template<tz::unique_cloneable_type T>
	maybe_owned_list<T>::maybe_owned_list(std::span<const T* const> ts)
	{
		for(const T* t : ts)
		{
			if(t == nullptr)
			{
				this->asset_storage.push_back(nullptr);
			}
			else
			{
				this->asset_storage.push_back(t->unique_clone());
			}
		}
	}

	template<tz::unique_cloneable_type T>
	unsigned int maybe_owned_list<T>::count() const
	{
		return this->asset_storage.size();
	}

	template<tz::unique_cloneable_type T>
	const T* maybe_owned_list<T>::get(handle_t handle) const
	{
		if(handle == tz::nullhand) return nullptr;
		std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
		return this->asset_storage[handle_val].get();
	}

	template<tz::unique_cloneable_type T>
	T* maybe_owned_list<T>::get(handle_t handle)
	{
		if(handle == tz::nullhand) return nullptr;
		std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
		return this->asset_storage[handle_val].get();
	}

	template<tz::unique_cloneable_type T>
	void maybe_owned_list<T>::set(handle_t handle, T* value)
	{
		std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
		tz::assert(!this->asset_storage[handle_val].owning(), "maybe_owned_list: Try to set specific asset value, but the asset at that handle is not a reference (it is owned by us)");
		this->asset_storage[handle_val] = value;
	}
}