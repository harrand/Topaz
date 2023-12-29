#ifndef TOPAZ_CORE_DATA_MAYBE_OWNED_LIST_HPP
#define TOPAZ_CORE_DATA_MAYBE_OWNED_LIST_HPP
#include "tz/core/types.hpp"
#include "tz/core/data/handle.hpp"
#include "tz/core/memory/maybe_owned_ptr.hpp"
#include <span>
#include <vector>

namespace tz
{
	template<tz::unique_cloneable_type T>
	class maybe_owned_list
	{
	public:
		using handle_t = tz::handle<T>;
		maybe_owned_list(std::span<const T* const> ts = {});

		unsigned int count() const;
		const T* get(handle_t handle) const;
		T* get(handle_t handle);
		void set(handle_t handle, T* value);
	private:
		std::vector<maybe_owned_ptr<T>> asset_storage;
	};

}

#include "tz/core/data/maybe_owned_list.inl"
#endif // TOPAZ_CORE_DATA_MAYBE_OWNED_LIST_HPP