#include "tz/core/types.hpp"
#include <cstdint>
#include <tuple>
#include <variant>

namespace tz
{
	/**
	 * @ingroup tz_core_algorithms
	 * Compute a compile-time for-loop.
	 * 
	 * Equivalent to:
	 * ```
	 * for(std::size_t i = begin; i < end; i++)
	 * {
	 * 	function(i);
	 * }
	 * ```
	 */
	template<std::size_t begin, std::size_t end>
	constexpr void static_for(tz::action<std::size_t> auto function)
	{
		if constexpr(begin < end)
		{
			function(std::integral_constant<std::size_t, begin>{});
			static_for<begin + 1, end>(function);
		}
	}

	/**
	 * @ingroup tz_core_algorithms
	 * Check if a needle is found in a haystack at compile-time.
	 * @tparam needle needle type to check exists within haystack...
	 * @tparam haystack Parameter pack which may or may not contain the type needle.
	 * @return True if the template parameter pack 'haystack' contains a type such that std::is_same<needle, Type> is true. Otherwise, returns false.
	 */
	template<typename needle, typename... haystack>
	constexpr bool static_find()
	{
		bool b = false;
		static_for<0, sizeof...(haystack)>([&b]([[maybe_unused]] auto i) constexpr
		{
			if constexpr(std::is_same<std::decay_t<decltype(std::get<i.value>(std::declval<std::tuple<haystack...>>()))>, needle>::value)
			{
				b = true;
			}
		});
		return b;
	}
}
