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
			function(std::integral_constant<std::size_t, Begin>{}.value);
			static_for<Begin + 1, End>(function);
		}
	}
}

#endif // TOPAZ_ALGO_STATIC_HPP