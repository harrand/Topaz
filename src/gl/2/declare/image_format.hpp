#ifndef TOPAZ_GL2_DECLARE_IMAGE_FORMAT_HPP
#define TOPAZ_GL2_DECLARE_IMAGE_FORMAT_HPP
#include "core/assert.hpp"
#include <climits>

namespace tz::gl2
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

		Depth16_UNorm 
	};

	constexpr std::size_t pixel_size_bytes(ImageFormat fmt)
	{
		static_assert(CHAR_BIT == 8, "Detected architecture where 1 byte != 8 bits. Topaz does not support this hardware.");
		switch(fmt)
		{
		using enum ImageFormat;
		case R8_UNorm:
		case R8_SNorm:
		case R8_UInt:
		case R8_SInt:
		case R8_sRGB:
			return 1;
		break;
		case R16_UNorm:
		case R16_SNorm:
		case R16_UInt:
		case R16_SInt:
		case RG16_UNorm:
		case RG16_SNorm:
		case RG16_UInt:
		case RG16_SInt:
		case RG16_sRGB:
		case Depth16_UNorm:
			return 2;
		break;
		case RGB24_UNorm:
		case RGB24_SNorm:
		case RGB24_UInt:
		case RGB24_SInt:
		case RGB24_sRGB:
		case BGR24_UNorm:
		case BGR24_SNorm:
		case BGR24_UInt:
		case BGR24_SInt:
		case BGR24_sRGB:
			return 3;
		break;
		case RG32_UNorm:
		case RG32_SNorm:
		case RG32_UInt:
		case RG32_SInt:
		case RGBA32_UNorm:
		case RGBA32_SNorm:
		case RGBA32_UInt:
		case RGBA32_SInt:
		case RGBA32_sRGB:
		case BGRA32_UNorm:
		case BGRA32_SNorm:
		case BGRA32_UInt:
		case BGRA32_SInt:
		case BGRA32_sRGB:
			return 4;
		break;
		default:
			tz_error("Unrecognised TZ ImageFormat");
			return 0;
		break;
		}
		return 0;
	}
}

#endif // TOPAZ_GL2_DECLARE_IMAGE_FORMAT_HPP
