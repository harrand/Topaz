#ifndef TOPAZ_CORE_MEMORY_HPP
#define TOPAZ_CORE_MEMORY_HPP
#include <span>
#include <ranges>
#include <type_traits>

namespace tz
{
	template<typename T> requires(std::is_standard_layout_v<T>)
	std::span<const std::byte> view_bytes(const T& t)
	{
		return std::as_bytes(std::span<const T>(&t, 1));
	}

	template<typename T> requires(std::is_standard_layout_v<T>)
	std::span<std::byte> view_bytes(T& t)
	{
		return std::as_writable_bytes(std::span<T>(&t, 1));
	}

	template<std::ranges::contiguous_range R> requires(std::is_const_v<std::ranges::range_value_t<R>>)
	std::span<const std::byte> view_bytes(R&& range)
	{
		using T = std::ranges::range_value_t<R>;
		return std::as_bytes(std::span<const T>(std::ranges::begin(range), std::ranges::end(range)));
	}

	template<std::ranges::contiguous_range R>
	std::span<std::byte> view_bytes(R&& range)
	{
		using T = std::ranges::range_value_t<R>;
		return std::as_writable_bytes(std::span<T>(std::ranges::begin(range), std::ranges::end(range)));
	}
}

#endif // TOPAZ_CORE_MEMORY_HPP