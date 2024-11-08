#ifndef TOPAZ_CORE_MEMORY_HPP
#define TOPAZ_CORE_MEMORY_HPP
#include <span>
#include <ranges>
#include <type_traits>

namespace tz
{
	template<typename T>
	constexpr auto view_bytes(T&& t) -> std::conditional_t<std::is_const_v<T>, std::span<const std::byte>, std::span<std::byte>>
	{
		if constexpr(requires{std::ranges::contiguous_range<T>;})
		{
			// we are a range	
			using V = std::ranges::range_value_t<T>;
			std::span<V> range{std::begin(t), std::end(t)};
			if constexpr(std::is_const_v<V>)
			{
				return std::as_bytes(range);
			}
			else
			{
				return std::as_writable_bytes(range);
			}
		}
		else
		{
			// we are not a range
			static_assert(std::is_standard_layout_v<T>, "view_bytes must be provided a contiguous range or a standard-layout-type. You have provided neither.");
			// we are a scalar-y value
			std::span<T, 1> span{&t};
			if constexpr(std::is_const_v<T>)
			{
				return std::as_bytes(span);
			}
			else
			{
				return std::as_writable_bytes(span);
			}
		}
	}
}

#endif // TOPAZ_CORE_MEMORY_HPP