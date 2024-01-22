#ifndef TOPAZ_CORE_TYPES_HPP
#define TOPAZ_CORE_TYPES_HPP
#include "tz/core/memory/memblk.hpp"
#include <type_traits>
#include <concepts>
#include <iterator>
#include <ranges>

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

	template<typename T>
	concept unique_cloneable_type = requires(T* t)
	{
		{t->unique_clone()};
	};

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
		{T::null()};
		{t.is_null()} -> std::same_as<bool>;
	};

	template<typename T>
	concept native_available = requires(T t)
	{
		T::NativeType;
		{t.native()} -> std::same_as<typename T::NativeType>;
	};

	template<typename T>
	concept allocator = requires(T t, tz::memblk blk, std::size_t sz)
	{
		{t.allocate(sz)} -> std::same_as<tz::memblk>;
		{t.deallocate(blk)} -> std::same_as<void>;
		{t.owns(blk)} -> std::same_as<bool>;
	};

	template<typename T>
	concept container = requires(T a, const T b)
	{
		requires std::regular<T>;
		requires std::swappable<T>;
		typename T::iterator;	
		requires std::destructible<typename T::value_type>;
		requires std::same_as<typename T::reference, typename T::value_type &>;
		requires std::same_as<typename T::const_reference, const typename T::value_type &>;
		requires std::forward_iterator<typename T::iterator>;
		requires std::forward_iterator<typename T::const_iterator>;
		requires std::signed_integral<typename T::difference_type>;
		requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::iterator>::difference_type>;
		requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::const_iterator>::difference_type>;
		{ a.begin() } -> std::same_as<typename T::iterator>;
		{ a.end() } -> std::same_as<typename T::iterator>;
		{ b.begin() } -> std::same_as<typename T::const_iterator>;
		{ b.end() } -> std::same_as<typename T::const_iterator>;
		{ a.cbegin() } -> std::same_as<typename T::const_iterator>;
		{ a.cend() } -> std::same_as<typename T::const_iterator>;
		{ a.size() } -> std::same_as<typename T::size_type>;
		{ a.max_size() } -> std::same_as<typename T::size_type>;
		{ a.empty() } -> std::same_as<bool>;
	};

	template<typename T>
	concept random_access_container = requires(T a, const T b)
	{
		requires tz::container<T>;
		requires std::random_access_iterator<typename T::iterator>;
	};

	template<typename T>
	concept message = requires(T a)
	{
		requires std::is_standard_layout_v<T>;
	};

	template<typename R, typename T>
	concept range_of = std::ranges::range<R> && std::same_as<std::decay_t<std::ranges::range_value_t<R>>, std::decay_t<T>>; 
}

#endif // TOPAZ_CORE_TYPES_HPP
