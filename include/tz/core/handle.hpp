#ifndef TZ_DATA_HANDLE_HPP
#define TZ_DATA_HANDLE_HPP
#include <cstdint>
#include <utility>
#include <limits>

namespace tz
{
	enum class hanval : std::uint32_t{};

	struct nullhand_t
	{
		bool operator==(std::integral auto num) const
		{
			return std::cmp_equal(num, std::numeric_limits<std::underlying_type_t<hanval>>::max());
		}
	};
	/**
	 * @ingroup tz_core
	 * Represents the null handle. You can assign any @ref tz::handle<T> to the null handle, meaning it no longer corresponds to something valid. You can also compare any handle to the null handle.
	**/
	constexpr nullhand_t nullhand;

	/**
	 * @ingroup tz_core
	 * @brief Represents a generic opaque handle.
	 *
	 * To create your own opaque handle types, simply create a type alias for a `handle<T>` where `T` is any unique type identifier.
	**/
	template<typename T>
	class handle
	{
	public:
		handle(hanval v):
		value(v){}

		handle([[maybe_unused]] nullhand_t nh = {}):
		value(static_cast<hanval>(std::numeric_limits<std::underlying_type_t<hanval>>::max())){}

		explicit operator hanval() const
		{
			return this->value;
		}

		/// Disregard the concept of "opaqueness" of an opaque handle by peeking at the underlying value. You should avoid the use of this outside of debug tools.
		std::underlying_type_t<hanval> peek() const
		{
			return static_cast<std::underlying_type_t<hanval>>(this->value);
		}

		/// Assign a handle to the null handle, meaning it is now invalid.
		handle& operator=(hanval value)
		{
			this->value = value;
			return *this;
		}

		/// Compare a handle to the null handle. This comparison will return false if the handle is valid.
		bool operator==(nullhand_t) const
		{
			return handle<T>{nullhand}.value == this->value;
		}
		/// Compare a handle to the null handle. This comparison will return true if the handle is valid.
		bool operator!=(nullhand_t) const
		{
			return handle<T>{nullhand}.value != this->value;
		}

		bool operator==(const handle<T>& rhs) const = default;
		bool operator!=(const handle<T>& rhs) const = default;
	private:
		hanval value;
	};
}

#endif // TZ_DATA_HANDLE_HPP