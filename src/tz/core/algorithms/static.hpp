#ifndef TOPAZ_CORE_ALGORITHM_STATIC_HPP
#define TOPAZ_CORE_ALGORITHM_STATIC_HPP
#include <cstdint>
#include <tuple>
#include <variant>

namespace tz
{
	template<int F, int L>
	struct static_for_t
	{
		template<typename Functor>
		static inline constexpr void apply(const Functor& f)
		{
			if(F < L)
			{
				f(std::integral_constant<int, F>{});
				static_for_t<F + 1, L>::apply(f);
			}
		}

		template<typename Functor>
		inline constexpr void operator()(const Functor& f) const
		{
			apply(f);
		}
	};

	/**
	 * @ingroup tz_core_algorithms
	 * Compute a compile-time for-loop.
	 * Use `tz::static_for` as a value, i.e: `inline constexpr static_for_t static_for = {};`
	 * 
	 * Equivalent to:
	 * ```
	 * for(std::size_t i = begin; i < end; i++)
	 * {
	 * 	function(i);
	 * }
	 * ```
	 */
	template<int N>
	struct static_for_t<N, N>
	{
		template<typename Functor>
		static inline constexpr void apply([[maybe_unused]] const Functor& f){}
	};

	template<int F, int L>
	inline constexpr static_for_t<F, L> static_for = {};

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
#endif // TOPAZ_CORE_ALGORITHM_STATIC_HPP
