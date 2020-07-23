#ifndef TOPAZ_ALGO_STATIC_HPP
#define TOPAZ_ALGO_STATIC_HPP
#include <cstddef>
#include <tuple>

namespace tz::algo
{

	template<std::size_t Begin, std::size_t End, typename Functor>
	constexpr void static_for(const Functor& function)
	{
		if constexpr(Begin < End)
		{
			function(std::integral_constant<std::size_t, Begin>{});
			static_for<Begin + 1, End>(function);
		}
	}

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
}

#endif // TOPAZ_ALGO_STATIC_HPP