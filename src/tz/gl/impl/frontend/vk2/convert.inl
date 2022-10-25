#if TZ_VULKAN
#include <tuple>
#include <cstdint>
#include <optional>

namespace tz::gl
{
	using ImageFormatTuple = std::tuple<ImageFormat, tz::gl::vk2::ImageFormat>;
	using namespace tz::gl;

	constexpr ImageFormatTuple tuple_map[] =
	{
		{ImageFormat::Undefined, vk2::ImageFormat::Undefined},
		{ImageFormat::R8, vk2::ImageFormat::R8},
		{ImageFormat::R8_UNorm, vk2::ImageFormat::R8_UNorm},
		{ImageFormat::R8_SNorm, vk2::ImageFormat::R8_SNorm},
		{ImageFormat::R8_UInt, vk2::ImageFormat::R8_UInt},
		{ImageFormat::R8_SInt, vk2::ImageFormat::R8_SInt},
		{ImageFormat::R8_sRGB, vk2::ImageFormat::R8_sRGB},
		{ImageFormat::R16, vk2::ImageFormat::R16},
		{ImageFormat::R16_UNorm, vk2::ImageFormat::R16_UNorm},
		{ImageFormat::R16_SNorm, vk2::ImageFormat::R16_SNorm},
		{ImageFormat::R16_UInt, vk2::ImageFormat::R16_UInt},
		{ImageFormat::R16_SInt, vk2::ImageFormat::R16_SInt},
		{ImageFormat::RG16, vk2::ImageFormat::RG16},
		{ImageFormat::RG16_UNorm, vk2::ImageFormat::RG16_UNorm},
		{ImageFormat::RG16_SNorm, vk2::ImageFormat::RG16_SNorm},
		{ImageFormat::RG16_UInt, vk2::ImageFormat::RG16_UInt},
		{ImageFormat::RG16_SInt, vk2::ImageFormat::RG16_SInt},
		{ImageFormat::RG16_sRGB, vk2::ImageFormat::RG16_sRGB},
		{ImageFormat::RG32, vk2::ImageFormat::RG32},
		{ImageFormat::RG32_UNorm, vk2::ImageFormat::RG32_UNorm},
		{ImageFormat::RG32_SNorm, vk2::ImageFormat::RG32_SNorm},
		{ImageFormat::RG32_UInt, vk2::ImageFormat::RG32_UInt},
		{ImageFormat::RG32_SInt, vk2::ImageFormat::RG32_SInt},
		{ImageFormat::RGB24, vk2::ImageFormat::RGB24},
		{ImageFormat::RGB24_UNorm, vk2::ImageFormat::RGB24_UNorm},
		{ImageFormat::RGB24_SNorm, vk2::ImageFormat::RGB24_SNorm},
		{ImageFormat::RGB24_UInt, vk2::ImageFormat::RGB24_UInt},
		{ImageFormat::RGB24_SInt, vk2::ImageFormat::RGB24_SInt},
		{ImageFormat::RGB24_sRGB, vk2::ImageFormat::RGB24_sRGB},
		{ImageFormat::BGR24, vk2::ImageFormat::BGR24},
		{ImageFormat::BGR24_UNorm, vk2::ImageFormat::BGR24_UNorm},
		{ImageFormat::BGR24_SNorm, vk2::ImageFormat::BGR24_SNorm},
		{ImageFormat::BGR24_UInt, vk2::ImageFormat::BGR24_UInt},
		{ImageFormat::BGR24_SInt, vk2::ImageFormat::BGR24_SInt},
		{ImageFormat::BGR24_sRGB, vk2::ImageFormat::BGR24_sRGB},
		{ImageFormat::RGBA32, vk2::ImageFormat::RGBA32},
		{ImageFormat::RGBA32_UNorm, vk2::ImageFormat::RGBA32_UNorm},
		{ImageFormat::RGBA32_SNorm, vk2::ImageFormat::RGBA32_SNorm},
		{ImageFormat::RGBA32_UInt, vk2::ImageFormat::RGBA32_UInt},
		{ImageFormat::RGBA32_SInt, vk2::ImageFormat::RGBA32_SInt},
		{ImageFormat::RGBA32_sRGB, vk2::ImageFormat::RGBA32_sRGB},
		{ImageFormat::BGRA32, vk2::ImageFormat::BGRA32},
		{ImageFormat::BGRA32_UNorm, vk2::ImageFormat::BGRA32_UNorm},
		{ImageFormat::BGRA32_SNorm, vk2::ImageFormat::BGRA32_SNorm},
		{ImageFormat::BGRA32_UInt, vk2::ImageFormat::BGRA32_UInt},
		{ImageFormat::BGRA32_SInt, vk2::ImageFormat::BGRA32_SInt},
		{ImageFormat::BGRA32_sRGB, vk2::ImageFormat::BGRA32_sRGB},
		{ImageFormat::RGBA64_SFloat, vk2::ImageFormat::RGBA64_SFloat},
		{ImageFormat::RGBA128_SFloat, vk2::ImageFormat::RGBA128_SFloat},
		{ImageFormat::Depth16_UNorm, vk2::ImageFormat::Depth16_UNorm} 
	};

	template<typename ImageFormatType>
	constexpr std::optional<std::size_t> tuple_map_index_of(ImageFormatType type)
	{
		constexpr std::size_t map_len = sizeof(tuple_map) / sizeof(tuple_map[0]);
		for(std::size_t i = 0; i < map_len; i++)
		{
			if(std::get<ImageFormatType>(tuple_map[i]) == type)
			{
				return i;
			}
		}
		return std::nullopt;
	}

	constexpr vk2::ImageFormat to_vk2(ImageFormat fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<vk2::ImageFormat>(tuple_map[index.value()]);
		}
		return vk2::ImageFormat::Undefined;
	}

	constexpr ImageFormat from_vk2(vk2::ImageFormat fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<ImageFormat>(tuple_map[index.value()]);
		}
		return ImageFormat::Undefined;
	}
}

#endif // TZ_VULKAN
