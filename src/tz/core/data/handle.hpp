#ifndef TZ_DATA_HANDLE_HPP
#define TZ_DATA_HANDLE_HPP
#include "tz/core/types.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace tz
{
	enum class hanval : std::uint32_t{};

	struct nullhand_t
	{
		bool operator==(tz::number auto num) const
		{
			return std::cmp_equal(num, std::numeric_limits<std::underlying_type_t<hanval>>::max());
		}
	};
	constexpr nullhand_t nullhand;

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

		handle& operator=(hanval value)
		{
			this->value = value;
			return *this;
		}

		bool operator==(nullhand_t) const
		{
			return handle<T>{nullhand}.value == this->value;
		}
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
