#if TZ_VULKAN
#include <tuple>
#include <cstdint>
#include <optional>

namespace tz::gl
{
	using ImageFormatTuple = std::tuple<TextureFormat, vk::Image::Format>;

	constexpr ImageFormatTuple tuple_map[] =
	{
		{TextureFormat::Rgba32Signed, vk::Image::Format::Rgba32Signed},
		{TextureFormat::Rgba32Unsigned, vk::Image::Format::Rgba32Unsigned},
		{TextureFormat::Rgba32sRGB, vk::Image::Format::Rgba32sRGB},
		{TextureFormat::DepthFloat32, vk::Image::Format::DepthFloat32},
		{TextureFormat::Bgra32UnsignedNorm, vk::Image::Format::Bgra32UnsignedNorm}
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

	constexpr vk::Image::Format to_vk(TextureFormat fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<vk::Image::Format>(tuple_map[index.value()]);
		}
		return vk::Image::Format::Undefined;
	}

	constexpr TextureFormat from_vk(vk::Image::Format fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<TextureFormat>(tuple_map[index.value()]);
		}
		return TextureFormat::Null;
	}
}
#endif // TZ_VULKAN