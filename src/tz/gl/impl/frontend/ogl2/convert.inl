#if TZ_OGL
#include <tuple>
#include <cstdint>
#include <optional>

namespace tz::gl
{
	using ImageFormatTuple = std::tuple<ImageFormat, tz::gl::ogl2::ImageFormat>;
	using namespace tz::gl;

	constexpr ImageFormatTuple tuple_map[] =
	{
		{ImageFormat::Undefined, ogl2::ImageFormat::Undefined},
		{ImageFormat::R8, ogl2::ImageFormat::R8},
		{ImageFormat::R8_UNorm, ogl2::ImageFormat::R8_UNorm},
		{ImageFormat::R8_SNorm, ogl2::ImageFormat::R8_SNorm},
		{ImageFormat::R8_UInt, ogl2::ImageFormat::R8_UInt},
		{ImageFormat::R8_SInt, ogl2::ImageFormat::R8_SInt},
		{ImageFormat::R8_sRGB, ogl2::ImageFormat::R8_sRGB},
		{ImageFormat::R16, ogl2::ImageFormat::R16},
		{ImageFormat::R16_UNorm, ogl2::ImageFormat::R16_UNorm},
		{ImageFormat::R16_SNorm, ogl2::ImageFormat::R16_SNorm},
		{ImageFormat::R16_UInt, ogl2::ImageFormat::R16_UInt},
		{ImageFormat::R16_SInt, ogl2::ImageFormat::R16_SInt},
		{ImageFormat::RG16, ogl2::ImageFormat::RG16},
		{ImageFormat::RG16_UNorm, ogl2::ImageFormat::RG16_UNorm},
		{ImageFormat::RG16_SNorm, ogl2::ImageFormat::RG16_SNorm},
		{ImageFormat::RG16_UInt, ogl2::ImageFormat::RG16_UInt},
		{ImageFormat::RG16_SInt, ogl2::ImageFormat::RG16_SInt},
		{ImageFormat::RG16_sRGB, ogl2::ImageFormat::RG16_sRGB},
		{ImageFormat::RG32, ogl2::ImageFormat::RG32},
		{ImageFormat::RG32_UNorm, ogl2::ImageFormat::RG32_UNorm},
		{ImageFormat::RG32_SNorm, ogl2::ImageFormat::RG32_SNorm},
		{ImageFormat::RG32_UInt, ogl2::ImageFormat::RG32_UInt},
		{ImageFormat::RG32_SInt, ogl2::ImageFormat::RG32_SInt},
		{ImageFormat::RGB24, ogl2::ImageFormat::RGB24},
		{ImageFormat::RGB24_UNorm, ogl2::ImageFormat::RGB24_UNorm},
		{ImageFormat::RGB24_SNorm, ogl2::ImageFormat::RGB24_SNorm},
		{ImageFormat::RGB24_UInt, ogl2::ImageFormat::RGB24_UInt},
		{ImageFormat::RGB24_SInt, ogl2::ImageFormat::RGB24_SInt},
		{ImageFormat::RGB24_sRGB, ogl2::ImageFormat::RGB24_sRGB},
		{ImageFormat::BGR24, ogl2::ImageFormat::BGR24},
		{ImageFormat::BGR24_UNorm, ogl2::ImageFormat::BGR24_UNorm},
		{ImageFormat::BGR24_SNorm, ogl2::ImageFormat::BGR24_SNorm},
		{ImageFormat::BGR24_UInt, ogl2::ImageFormat::BGR24_UInt},
		{ImageFormat::BGR24_SInt, ogl2::ImageFormat::BGR24_SInt},
		{ImageFormat::BGR24_sRGB, ogl2::ImageFormat::BGR24_sRGB},
		{ImageFormat::RGBA32, ogl2::ImageFormat::RGBA32},
		{ImageFormat::RGBA32_UNorm, ogl2::ImageFormat::RGBA32_UNorm},
		{ImageFormat::RGBA32_SNorm, ogl2::ImageFormat::RGBA32_SNorm},
		{ImageFormat::RGBA32_UInt, ogl2::ImageFormat::RGBA32_UInt},
		{ImageFormat::RGBA32_SInt, ogl2::ImageFormat::RGBA32_SInt},
		{ImageFormat::RGBA32_sRGB, ogl2::ImageFormat::RGBA32_sRGB},
		{ImageFormat::BGRA32, ogl2::ImageFormat::BGRA32},
		{ImageFormat::BGRA32_UNorm, ogl2::ImageFormat::BGRA32_UNorm},
		{ImageFormat::BGRA32_SNorm, ogl2::ImageFormat::BGRA32_SNorm},
		{ImageFormat::BGRA32_UInt, ogl2::ImageFormat::BGRA32_UInt},
		{ImageFormat::BGRA32_SInt, ogl2::ImageFormat::BGRA32_SInt},
		{ImageFormat::BGRA32_sRGB, ogl2::ImageFormat::BGRA32_sRGB},
		{ImageFormat::RGBA64_SFloat, ogl2::ImageFormat::RGBA64_SFloat},
		{ImageFormat::RGBA128_SFloat, ogl2::ImageFormat::RGBA128_SFloat},
		{ImageFormat::Depth16_UNorm, ogl2::ImageFormat::Depth16_UNorm} 
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

	constexpr ogl2::ImageFormat to_ogl2(ImageFormat fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<ogl2::ImageFormat>(tuple_map[index.value()]);
		}
		return ogl2::ImageFormat::Undefined;
	}

	constexpr ImageFormat from_ogl2(ogl2::ImageFormat fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<ImageFormat>(tuple_map[index.value()]);
		}
		return ImageFormat::Undefined;
	}
}

#endif // TZ_OGL
