#ifndef TOPAZ_CORE_OPAQUE_HANDLE_HPP
#define TOPAZ_CORE_OPAQUE_HANDLE_HPP
#include <cstddef>
#include <type_traits>
#include <functional>
#include <limits>

namespace tz
{
	enum class HandleValue : std::size_t{};
	using HandleValueUnderlying = std::underlying_type_t<HandleValue>;

	struct nullhand_t{};
	/**
	 * @ingroup tz_core_utility
	 * Represents the null Handle. Convertible to any handle type.
	 */
	constexpr nullhand_t nullhand;

	/**
	 * @ingroup tz_core_utility
	 * Represents a generic opaque handle. Often engine features will return or take handles as parameters. Most likely you will want to save handles you are given when calling into engine routines (such as when adding resources to a renderer)
	 */
	template<typename T>
	class Handle
	{
	public:
		Handle(HandleValue value):
		value(value){}

		/**
		 * Retrieve the null handle.
		 */
		Handle(nullhand_t):
		value(static_cast<HandleValue>(std::numeric_limits<std::size_t>::max())){}

		explicit operator HandleValue() const
		{
			return this->value;
		}

		Handle& operator=(HandleValue value)
		{
			this->value = value;
			return *this;
		}
		bool operator==(nullhand_t) const
		{
			return this->value == static_cast<HandleValue>(std::numeric_limits<std::size_t>::max());
		}
		bool operator==(const Handle<T>& rhs) const = default;
	private:
		HandleValue value;
	};
}

template<typename T>
struct std::hash<tz::Handle<T>>
{
	std::size_t operator()(const tz::Handle<T>& handle) const
	{
		auto hand_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
		std::size_t ret = std::hash<int>{}(static_cast<int>(hand_val));
		return ret;
	}
};

#endif //TOPAZ_CORE_OPAQUE_HANDLE_HPP
