#ifndef TOPAZ_CORE_MEMORY_HPP
#define TOPAZ_CORE_MEMORY_HPP
#include <span>
#include <ranges>

namespace tz
{
	template<typename T>
	std::span<const std::byte> view_bytes(const T& t)
	{
		return std::as_bytes(std::span<const T>(&t, 1));
	}

	template<std::ranges::contiguous_range R>
	std::span<const std::byte> view_bytes(R&& range)
	{
		using T = std::ranges::range_value_t<R>;
		return std::as_bytes(std::span<const T>(std::ranges::begin(range), std::ranges::end(range)));
	}
}

#endif // TOPAZ_CORE_MEMORY_HPP