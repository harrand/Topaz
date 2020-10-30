#ifndef TOPAZ_ALGO_STATIC_HPP
#define TOPAZ_ALGO_STATIC_HPP
#include <cstddef>
#include <tuple>

namespace tz::algo
{
	/**
	* \addtogroup tz_algo Topaz Algorithms Library (tz::algo)
	* Contains common algorithms used in Topaz modules that aren't available in the C++ standard library.
	* @{
	*/

	/**
	 * Invoke the given functor with a single std::size_t parameter End - Begin times, incrementing the parameter each time.
	 * This should be used to iterate at compile-time.
	 * @tparam Begin Value at first iteration.
	 * @tparam End Value at last iteration.
	 * @tparam Functor Functor type to invoke. Must be of the signature X(std::size_t) where X is any type. This is recommended to be void as the return value will always be discarded.
	 * @param function Specific function to invoke. It's signature must match the tparam Functor.
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
	 * Query as to whether the given type is move-constructible and move-assignable.
	 * @tparam T Type to query.
	 * @return True if T is move-constructible and move-assignable.
	 */
	template<class T>
	constexpr bool moveable()
	{
		return std::is_move_constructible_v<T>
			&& std::is_move_assignable_v<T>;
	}

	/**
	 * Query as to whether the given type is copy-constructible and copt-assignable.
	 * @tparam T Type to query.
	 * @return True if T is copy-constructible and copy-assignable.
	 */
	template<class T>
	constexpr bool copyable()
	{
		return std::is_copy_constructible_v<T>
			&& std::is_copy_assignable_v<T>;
	}

	namespace detail
	{
		struct No {};

		template<class T, class Operator>
		struct ExistsOperator
		{
			enum { value = !std::is_same<decltype(std::declval<Operator>()(std::declval<T>(), std::declval<T>())), No>::value };
		};
	}

	/**
	 * Query as to whether T::operator==(T) exists or something very similar.
	 * @tparam T Type to query equatable
	 * @return True if T can be compared with another T, otherwise false.
	 */
	template<class T>
	constexpr bool equality_comparable()
	{
		return detail::ExistsOperator<T, std::equal_to<T>>::value;
	}

	/**
	 * @}
	 */
}

#endif // TOPAZ_ALGO_STATIC_HPP