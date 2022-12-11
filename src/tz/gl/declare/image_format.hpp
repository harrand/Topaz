#ifndef TOPAZ_GL2_DECLARE_IMAGE_FORMAT_HPP
#define TOPAZ_GL2_DECLARE_IMAGE_FORMAT_HPP
#include "hdk/debug.hpp"
#include <climits>
#include <array>

namespace tz::gl
{
	/**
	 * @ingroup tz_gl2
	 * Various image formats are supported in Topaz.
	 *
	 * ImageFormats are comprised of three properties.
	 *
	 * The enum values are named as `Components``Size``Internal` where:
	 * - Components (R, RG, RGB, BGR, RGBA): Number of components and order.
	 * - Size (8, 16, 24, 32): Total size of the whole element, not just one component. Assume all components are equally-sized.
	 * - Internal (UNORM, SNORM, UINT, SINT): Value type of the element. This can be ommitted, in which case the default is UNORM.
	 * 	- UNORM = float [0 -> -1]
	 * 	- SNORM = float [-1 -> 1]
	 * 	- UINT = unsigned int
	 * 	- SINT = signed int
	 * 	- SFLOAT = signed float
	 * 	- SRGB = sRGB nonlinear encoding
	 *
	 * Examples:
	 * - RGBA32 = (RGBA, 32 bit, UNORM)
	 * - RGBA32_UNorm = (RGBA, 32 bit, UNORM) == RGBA32
	 * - R8_SNorm = (R, 8 bit, SNORM)
	 * - BGRA16_UInt = (BGRA, 16 bit, UINT)
	 */
	enum class ImageFormat
	{
		/// - Undefined Format. It is mostly an error to use this.
		Undefined,
		R8,
		R8_UNorm = R8,
		R8_SNorm,
		R8_UInt,
		R8_SInt,
		R8_sRGB,
		R16,
		R16_UNorm = R16,
		R16_SNorm,
		R16_UInt,
		R16_SInt,
		RG16,
		RG16_UNorm = RG16,
		RG16_SNorm,
		RG16_UInt,
		RG16_SInt,
		RG16_sRGB,
		RG32,
		RG32_UNorm = RG32,
		RG32_SNorm,
		RG32_UInt,
		RG32_SInt,
		RGB24,
		RGB24_UNorm = RGB24,
		RGB24_SNorm,
		RGB24_UInt,
		RGB24_SInt,
		RGB24_sRGB,
		BGR24,
		BGR24_UNorm = BGR24,
		BGR24_SNorm,
		BGR24_UInt,
		BGR24_SInt,
		BGR24_sRGB,
		RGBA32,
		RGBA32_UNorm = RGBA32,
		RGBA32_SNorm,
		RGBA32_UInt,
		RGBA32_SInt,
		RGBA32_sRGB,
		BGRA32,
		BGRA32_UNorm = BGRA32,
		BGRA32_SNorm,
		BGRA32_UInt,
		BGRA32_SInt,
		BGRA32_sRGB,
		RGBA64_SFloat,
		RGBA128_SFloat,
		Depth16,
		Depth16_UNorm = Depth16,
		Count
	};

	constexpr bool is_depth_format(ImageFormat fmt)
	{
		switch(fmt)
		{
			case ImageFormat::Depth16:
			[[fallthrough]];
				return true;
			break;
			default:
				return false;
			break;
		}
	}

	constexpr std::size_t pixel_size_bytes(ImageFormat fmt)
	{
		static_assert(CHAR_BIT == 8, "Detected architecture where 1 byte != 8 bits. Topaz does not support this hardware.");
		switch(fmt)
		{
		case ImageFormat::R8_UNorm:
		case ImageFormat::R8_SNorm:
		case ImageFormat::R8_UInt:
		case ImageFormat::R8_SInt:
		case ImageFormat::R8_sRGB:
			return 1;
		break;
		case ImageFormat::R16_UNorm:
		case ImageFormat::R16_SNorm:
		case ImageFormat::R16_UInt:
		case ImageFormat::R16_SInt:
		case ImageFormat::RG16_UNorm:
		case ImageFormat::RG16_SNorm:
		case ImageFormat::RG16_UInt:
		case ImageFormat::RG16_SInt:
		case ImageFormat::RG16_sRGB:
		case ImageFormat::Depth16_UNorm:
			return 2;
		break;
		case ImageFormat::RGB24_UNorm:
		case ImageFormat::RGB24_SNorm:
		case ImageFormat::RGB24_UInt:
		case ImageFormat::RGB24_SInt:
		case ImageFormat::RGB24_sRGB:
		case ImageFormat::BGR24_UNorm:
		case ImageFormat::BGR24_SNorm:
		case ImageFormat::BGR24_UInt:
		case ImageFormat::BGR24_SInt:
		case ImageFormat::BGR24_sRGB:
			return 3;
		break;
		case ImageFormat::RG32_UNorm:
		case ImageFormat::RG32_SNorm:
		case ImageFormat::RG32_UInt:
		case ImageFormat::RG32_SInt:
		case ImageFormat::RGBA32_UNorm:
		case ImageFormat::RGBA32_SNorm:
		case ImageFormat::RGBA32_UInt:
		case ImageFormat::RGBA32_SInt:
		case ImageFormat::RGBA32_sRGB:
		case ImageFormat::BGRA32_UNorm:
		case ImageFormat::BGRA32_SNorm:
		case ImageFormat::BGRA32_UInt:
		case ImageFormat::BGRA32_SInt:
		case ImageFormat::BGRA32_sRGB:
			return 4;
		break;
		case ImageFormat::RGBA64_SFloat:
			return 8;
		break;
		case ImageFormat::RGBA128_SFloat:
			return 16;
		break;
		default:
			hdk::error("Unrecognised TZ ImageFormat");
			return 0;
		break;
		}
		return 0;
	}

	namespace detail
	{
		constexpr std::array<const char*, static_cast<int>(ImageFormat::Count)> image_format_strings =
		{
			"Undefined",
			"R8",
			"R8_SNorm",
			"R8_UInt",
			"R8_SInt",
			"R8_sRGB",
			"R16",
			"R16_SNorm",
			"R16_UInt",
			"R16_SInt",
			"RG16",
			"RG16_SNorm",
			"RG16_UInt",
			"RG16_SInt",
			"RG16_sRGB",
			"RG32_UNorm",
			"RG32_SNorm",
			"RG32_UInt",
			"RG32_SInt",
			"RGB24",
			"RGB24_SNorm",
			"RGB24_UInt",
			"RGB24_SInt",
			"RGB24_sRGB",
			"BGR24",
			"BGR24_SNorm",
			"BGR24_UInt",
			"BGR24_SInt",
			"BGR24_sRGB",
			"RGBA32",
			"RGBA32_SNorm",
			"RGBA32_UInt",
			"RGBA32_SInt",
			"RGBA32_sRGB",
			"BGRA32",
			"BGRA32_SNorm",
			"BGRA32_UInt",
			"BGRA32_SInt",
			"BGRA32_sRGB",
			"RGBA64_SFloat",
			"RGBA128_SFloat",
			"Depth16",
		};
	}
}

#endif // TOPAZ_GL2_DECLARE_IMAGE_FORMAT_HPP
