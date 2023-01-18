#ifndef TOPAZ_CORE_TYPES_HPP
#define TOPAZ_CORE_TYPES_HPP
#include <type_traits>
#include <concepts>
#include "tz/core/memory.hpp"

namespace tz
{
	/**
	 * An integral type or a floating-point type (including char and bool).
	 */
	template<typename T>
	concept arithmetic = std::is_arithmetic_v<std::decay_t<T>>;

	template<typename T>
	concept const_type = std::is_const_v<T>;

	template<typename T>
	concept enum_class = std::is_enum_v<T> && !std::is_convertible_v<T, int>;

	/**
	 * A number is any arithmetic type excluding char and bool.
	 */
	template<typename T>
	concept number = requires
	{
		requires arithmetic<T>;
		requires !std::is_same_v<std::remove_cvref_t<T>, bool>;
		requires !std::is_same_v<std::remove_cvref_t<T>, char>;
	};

	/**
	 * function concept which constrains F to any type that can be called with `Args...` to produce a Result.
	 * 
	 * This is backend-agnostic, meaning that any invocable backend (func ptr, lambda, functor, you name it...) can be used so long as it matches the signature Result(Args...).
	 */
	template<typename F, typename Result, typename... Args>
	concept function = requires(F f, Args... args)
	{
		{f(args...)} -> std::convertible_to<Result>;
	};

	/**
	 * Just like a tz::function, except will never return anything. This makes for cleaner syntax.
	 */
	template<typename F, typename... Args>
	concept action = function<F, void, Args...>;

	template<typename T>
	constexpr bool copyable = requires { requires std::copyable<T>; };

	template<typename T>
	constexpr bool moveable = requires { requires std::movable<T>; };

	template<typename T>
	concept trivially_copyable = std::is_trivially_copyable_v<T>;

	template<typename T>
	concept nullable = requires(const T t)
	{
		//{decltype(t)::null()};
		{t.is_null()} -> std::same_as<bool>;
	};

	template<typename T>
	concept native_available = requires(T t)
	{
		T::NativeType;
		{t.native()} -> std::same_as<typename T::NativeType>;
	};
}

#endif // TOPAZ_CORE_TYPES_HPP
