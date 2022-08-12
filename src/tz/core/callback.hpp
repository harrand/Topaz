#ifndef TOPAZ_CORE_CALLBACK_HPP
#define TOPAZ_CORE_CALLBACK_HPP
#include "tz/core/handle.hpp"
#include "tz/core/types.hpp"
#include <vector>
#include <functional>

namespace tz
{
	namespace detail
	{
		struct CallbackType{};
	}
	/**
	 * @ingroup tz_core_utility
	 * Opaque handle representing a reference to an existing @ref tz::Callback function.
	 */
	using CallbackHandle = tz::Handle<detail::CallbackType>;

	/**
	 * @ingroup tz_core_utility
	 * Represents a centralised storage/management of callback functions of a common signature `void(Args...)`.
	 *
	 * Callables matching the above signature can be registered by this object. When registered the caller is provided a reference handle. The token can be used to unregister the callback later.
	 *
	 * If a callback object is invoked with some arguments, each registered callable is also invoked with the same arguments. It is only recommended to use callbacks very sparingly -- Most often usage of callbacks are wrong. The engine itself uses callbacks extremely rarely. If you would like a cross-module messaging system for example, do not use these callbacks; create your own functionality for this.
	 *
	 */
	template<typename... Args>
	class Callback
	{
	public:
		/**
		 * Create an empty callback object with no registered callables.
		 */
		Callback() = default;
		/**
		 * Register a new callable with this callback object. The callable is assumed to remain valid until either the destruction of this callback object, or until it is manually deregistered.
		 * @param callback_function Callable matching the given signature.
		 * @return CallbackHandle corresponding to the registered callback. Cache this handle and use it to deregister the callable via @ref remove_callback once the lifetime of the provided callable reaches its end or the functionality is no longer required.
		 */
		CallbackHandle add_callback(tz::Action<Args...> auto&& callback_function)
		{
			this->callback_storage.push_back(callback_function);
			return static_cast<tz::HandleValue>(this->callback_storage.size() - 1);
		}
		/**
		 * Deregister a callable that was registered by this callback object at some point in the past. The corresponding callable will no longer be invoked when the callback object is invoked.
		 *
		 * @param handle CallbackHandle corresponding to the result of a previous @ref add_callback invocation.
		 */
		void remove_callback(CallbackHandle handle)
		{
			std::size_t handle_value = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
			this->callback_storage[handle_value] = nullptr;
		}
		/**
		 * Invoke the callback object, calling all registered callbacks with the provided aerguments.
		 * @param args Argument values to pass to each registered callable.
		 */
		void operator()(Args... args) const
		{
			for(const auto& callback_func : this->callback_storage)
			{
				if(callback_func != nullptr)
				{
					callback_func(std::forward<Args>(args)...);
				}
			}
		}

		bool empty() const
		{
			return this->callback_storage.empty();
		}
	private:
		std::vector<std::function<void(Args...)>> callback_storage = {};
	};
}

#endif // TOPAZ_CORE_CALLBACK_HPP
