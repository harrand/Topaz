#if TZ_OGL
#include <tuple>
#include <cstdint>
#include <optional>

namespace tz::gl
{
	using ImageFormatTuple = std::tuple<TextureFormat, ogl::Texture::Format>;

	constexpr ImageFormatTuple tuple_map[] =
	{
		{TextureFormat::Rgba32Signed, ogl::Texture::Format::Rgba32Signed},
		{TextureFormat::Rgba32Unsigned, ogl::Texture::Format::Rgba32Unsigned},
		{TextureFormat::Rgba32sRGB, ogl::Texture::Format::Rgba32sRGB},
		{TextureFormat::DepthFloat32, ogl::Texture::Format::DepthFloat32},
		{TextureFormat::Bgra32UnsignedNorm, ogl::Texture::Format::Bgra32UnsignedNorm}
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

	constexpr ogl::Texture::Format to_ogl(TextureFormat fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<ogl::Texture::Format>(tuple_map[index.value()]);
		}
		return ogl::Texture::Format::Undefined;
	}

	constexpr TextureFormat from_ogl(ogl::Texture::Format fmt)
	{
		std::optional<std::size_t> index = tuple_map_index_of(fmt);
		if(index.has_value())
		{
			return std::get<TextureFormat>(tuple_map[index.value()]);
		}
		return TextureFormat::Null;
	}
}
#endif // TZ_OGL