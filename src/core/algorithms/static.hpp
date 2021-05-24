#include <cstdint>
#include <tuple>
#include <variant>

namespace tz
{
	/**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
	/**
	 * Compute a compile-time for-loop.
	 */
    template<std::size_t Begin, std::size_t End, typename Functor>
	constexpr void static_for(const Functor& function)
	{
		if constexpr(Begin < End)
		{
			function(std::integral_constant<std::size_t, Begin>{});
			static_for<Begin + 1, End>(function);
		}
	}

	/**
	 * Check if a Needle is found in a Haystack at compile-time.
	 * @tparam Needle Needle type to check exists within Haystack...
	 * @tparam Haystack Parameter pack which may or may not contain the type Needle.
	 * @return True if the template parameter pack 'Haystack' contains a type such that std::is_same<Needle, Type> is true. Otherwise, returns false.
	 */
	template<typename Needle, typename... Haystack>
	constexpr bool static_find()
	{
		bool b = false;
		static_for<0, sizeof...(Haystack)>([&b]([[maybe_unused]] auto i) constexpr
		{
			if constexpr(std::is_same<std::decay_t<decltype(std::get<i.value>(std::declval<std::tuple<Haystack...>>()))>, Needle>::value)
			{
				b = true;
			}
		});
		return b;
	}
	/**
	 * @}
	 */
}