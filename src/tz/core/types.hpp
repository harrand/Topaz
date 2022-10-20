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
	concept Arithmetic = std::is_arithmetic_v<std::decay_t<T>>;

	template<typename T>
	concept ConstType = std::is_const_v<T>;

	template<typename T>
	concept EnumClass = std::is_enum_v<T> && !std::is_convertible_v<T, int>;

	/**
	 * A number is any arithmetic type excluding char and bool.
	 */
	template<typename T>
	concept Number = requires
	{
		requires Arithmetic<T>;
		requires !std::is_same_v<std::remove_cvref_t<T>, bool>;
		requires !std::is_same_v<std::remove_cvref_t<T>, char>;
	};

	/**
	 * Function concept which constrains F to any type that can be called with `Args...` to produce a Result.
	 * 
	 * This is backend-agnostic, meaning that any invocable backend (func ptr, lambda, functor, you name it...) can be used so long as it matches the signature Result(Args...).
	 */
	template<typename F, typename Result, typename... Args>
	concept Function = requires(F f, Args... args)
	{
		{f(args...)} -> std::convertible_to<Result>;
	};

	/**
	 * Just like a tz::Function, except will never return anything. This makes for cleaner syntax.
	 */
	template<typename F, typename... Args>
	concept Action = Function<F, void, Args...>;

	template<typename T>
	constexpr bool copyable = requires { requires std::copyable<T>; };

	template<typename T>
	constexpr bool moveable = requires { requires std::movable<T>; };

	template<typename T>
	concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

	template<typename T>
	concept Nullable = requires(const T t)
	{
		//{decltype(t)::null()};
		{t.is_null()} -> std::same_as<bool>;
	};

	template<typename T>
	concept NativeAvailable = requires(T t)
	{
		T::NativeType;
		{t.native()} -> std::same_as<typename T::NativeType>;
	};

	/**
	 * @ingroup tz_core_allocators
	 * Named requirement for topaz allocators. Not to be confused with the c++ standard library allocators.
	 */
	template<typename T>
	concept Allocator = requires(T t, tz::Blk blk, std::size_t sz)
	{
		{t.allocate(sz)} -> std::same_as<tz::Blk>;
		{t.deallocate(blk)} -> std::same_as<void>;
		{t.owns(blk)} -> std::same_as<bool>;
	};
}

#endif // TOPAZ_CORE_TYPES_HPP
