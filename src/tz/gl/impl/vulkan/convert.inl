#if TZ_VULKAN
#include <tuple>
#include <cstdint>
#include <optional>

namespace tz::gl
{
	using image_formatTuple = std::tuple<image_format, tz::gl::vk2::image_format>;
	using namespace tz::gl;

	constexpr image_formatTuple tuple_map[] =
	{
		{image_format::undefined, vk2::image_format::undefined},
		{image_format::R8, vk2::image_format::R8},
		{image_format::R8_UNorm, vk2::image_format::R8_UNorm},
		{image_format::R8_SNorm, vk2::image_format::R8_SNorm},
		{image_format::R8_UInt, vk2::image_format::R8_UInt},
		{image_format::R8_SInt, vk2::image_format::R8_SInt},
		{image_format::R8_sRGB, vk2::image_format::R8_sRGB},
		{image_format::R16, vk2::image_format::R16},
		{image_format::R16_UNorm, vk2::image_format::R16_UNorm},
		{image_format::R16_SNorm, vk2::image_format::R16_SNorm},
		{image_format::R16_UInt, vk2::image_format::R16_UInt},
		{image_format::R16_SInt, vk2::image_format::R16_SInt},
		{image_format::RG16, vk2::image_format::RG16},
		{image_format::RG16_UNorm, vk2::image_format::RG16_UNorm},
		{image_format::RG16_SNorm, vk2::image_format::RG16_SNorm},
		{image_format::RG16_UInt, vk2::image_format::RG16_UInt},
		{image_format::RG16_SInt, vk2::image_format::RG16_SInt},
		{image_format::RG16_sRGB, vk2::image_format::RG16_sRGB},
		{image_format::RG32, vk2::image_format::RG32},
		{image_format::RG32_UNorm, vk2::image_format::RG32_UNorm},
		{image_format::RG32_SNorm, vk2::image_format::RG32_SNorm},
		{image_format::RG32_UInt, vk2::image_format::RG32_UInt},
		{image_format::RG32_SInt, vk2::image_format::RG32_SInt},
		{image_format::RGB24, vk2::image_format::RGB24},
		{image_format::RGB24_UNorm, vk2::image_format::RGB24_UNorm},
		{image_format::RGB24_SNorm, vk2::image_format::RGB24_SNorm},
		{image_format::RGB24_UInt, vk2::image_format::RGB24_UInt},
		{image_format::RGB24_SInt, vk2::image_format::RGB24_SInt},
		{image_format::RGB24_sRGB, vk2::image_format::RGB24_sRGB},
		{image_format::BGR24, vk2::image_format::BGR24},
		{image_format::BGR24_UNorm, vk2::image_format::BGR24_UNorm},
		{image_format::BGR24_SNorm, vk2::image_format::BGR24_SNorm},
		{image_format::BGR24_UInt, vk2::image_format::BGR24_UInt},
		{image_format::BGR24_SInt, vk2::image_format::BGR24_SInt},
		{image_format::BGR24_sRGB, vk2::image_format::BGR24_sRGB},
		{image_format::RGBA32, vk2::image_format::RGBA32},
		{image_format::RGBA32_UNorm, vk2::image_format::RGBA32_UNorm},
		{image_format::RGBA32_SNorm, vk2::image_format::RGBA32_SNorm},
		{image_format::RGBA32_UInt, vk2::image_format::RGBA32_UInt},
		{image_format::RGBA32_SInt, vk2::image_format::RGBA32_SInt},
		{image_format::RGBA32_sRGB, vk2::image_format::RGBA32_sRGB},
		{image_format::BGRA32, vk2::image_format::BGRA32},
		{image_format::BGRA32_UNorm, vk2::image_format::BGRA32_UNorm},
		{image_format::BGRA32_SNorm, vk2::image_format::BGRA32_SNorm},
		{image_format::BGRA32_UInt, vk2::image_format::BGRA32_UInt},
		{image_format::BGRA32_SInt, vk2::image_format::BGRA32_SInt},
		{image_format::BGRA32_sRGB, vk2::image_format::BGRA32_sRGB},
		{image_format::RGBA64_SFloat, vk2::image_format::RGBA64_SFloat},
		{image_format::RGBA128_SFloat, vk2::image_format::RGBA128_SFloat},
		{image_format::Depth16_UNorm, vk2::image_format::Depth16_UNorm} 
	};

	template<typename image_formatType>
	constexpr std::optional<std::size_t> tuple_map_index_of(image_formatType type)
	{
		constexpr std::size_t map_len = sizeof(tuple_map) / sizeof(tuple_map[0]);
		for(std::size_t i = 0; i < map_len; i++)
		{
			if(std::get<image_formatType>(tuple_map[i]) == type)
			{
				return i;
			}
		}
		return std::nullopt;
	}

	constexpr vk2::image_format to_vk2(image_format fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<vk2::image_format>(tuple_map[index.value()]);
		}
		return vk2::image_format::undefined;
	}

	constexpr image_format from_vk2(vk2::image_format fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<image_format>(tuple_map[index.value()]);
		}
		return image_format::undefined;
	}
}

#endif // TZ_VULKAN
