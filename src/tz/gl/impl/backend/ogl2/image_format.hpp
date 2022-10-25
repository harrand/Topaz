#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_FORMAT_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_FORMAT_HPP
#include "tz/gl/impl/backend/ogl2/tz_opengl.hpp"
#include <array>

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_image
	 * OpenGL formats are pretty finnicky, especially when using them to initialise texture data-stores. For this reason this helper struct exists which should encapsulate equivalent state which we will want to use in the frontend.
	 */
	struct FormatData
	{
		GLenum format;
		GLenum internal_format;
		GLenum type;
	};

	enum class ImageFormat
	{
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

		Depth16_UNorm,
		Depth32_UNorm,
		Count
	};

	constexpr std::array<FormatData, static_cast<int>(ImageFormat::Count)> internal_fmts
	{{
		/* Undefined */ {.format = GL_INVALID_VALUE, .internal_format = GL_INVALID_VALUE, .type = GL_INVALID_VALUE},
		/* R8, R8_UNorm */ {.format = GL_RED, .internal_format = GL_R8, .type = GL_UNSIGNED_BYTE},
		/* R8_SNorm */ {.format = GL_RED, .internal_format = GL_R8_SNORM, .type = GL_BYTE},
		/* R8_UInt */ {.format = GL_RED, .internal_format = GL_R8UI, .type = GL_UNSIGNED_INT},
		/* R8_SInt */ {.format = GL_RED, .internal_format = GL_R8I, .type = GL_INT},
		/* R8_sRGB */ {.format = GL_RED, .internal_format = GL_SRGB8, .type = GL_INT},

		/* R16, R16_UNorm */ {.format = GL_RED, .internal_format = GL_R16, .type = GL_UNSIGNED_BYTE},
		/* R16_SNorm */ {.format = GL_RED, .internal_format = GL_R16_SNORM, .type = GL_BYTE},
		/* R16_UInt */ {.format = GL_RED, .internal_format = GL_R16UI, .type = GL_UNSIGNED_INT},
		/* R16_SInt */ {.format = GL_RED, .internal_format = GL_R16I, .type = GL_INT},

		/* RG16, RG16_UNorm */ {.format = GL_RG, .internal_format = GL_RG8, .type = GL_UNSIGNED_BYTE},
		/* RG16_SNorm */ {.format = GL_RG, .internal_format = GL_RG8_SNORM, .type = GL_BYTE},
		/* RG16_UInt */ {.format = GL_RG, .internal_format = GL_RG8UI, .type = GL_UNSIGNED_INT},
		/* RG16_SInt */ {.format = GL_RG, .internal_format = GL_RG8I, .type = GL_INT},
		/* RG16_sRGB */ {.format = GL_RG, .internal_format = GL_SRGB8, .type = GL_INT},

		/* RG32, RG32_UNorm */ {.format = GL_RG, .internal_format = GL_RG16, .type = GL_UNSIGNED_BYTE},
		/* RG32_SNorm */ {.format = GL_RG, .internal_format = GL_RG16_SNORM, .type = GL_BYTE},
		/* RG32_UInt */ {.format = GL_RG, .internal_format = GL_RG16UI, .type = GL_UNSIGNED_INT},
		/* RG32_SInt */ {.format = GL_RG, .internal_format = GL_RG16I, .type = GL_INT},

		/* RGB24, RGB24_UNorm */ {.format = GL_RGB, .internal_format = GL_RGB8, .type = GL_UNSIGNED_BYTE},
		/* RGB24_SNorm */ {.format = GL_RGB, .internal_format = GL_RGB8_SNORM, .type = GL_BYTE},
		/* RGB24_UInt */ {.format = GL_RGB, .internal_format = GL_RGB8UI, .type = GL_UNSIGNED_INT},
		/* RGB24_SInt */ {.format = GL_RGB, .internal_format = GL_RGB8I, .type = GL_UNSIGNED_INT},
		/* RGB24_sRGB */ {.format = GL_RGB, .internal_format = GL_SRGB8, .type = GL_INT},

		/* BGR24, BGR24_UNorm */ {.format = GL_BGR, .internal_format = GL_RGB8, .type = GL_UNSIGNED_BYTE},
		/* BGR24_SNorm */ {.format = GL_BGR, .internal_format = GL_RGB8_SNORM, .type = GL_BYTE},
		/* BGR24_UInt */ {.format = GL_BGR, .internal_format = GL_RGB8UI, .type = GL_UNSIGNED_INT},
		/* BGR24_SInt */ {.format = GL_BGR, .internal_format = GL_RGB8I, .type = GL_INT},
		/* BGR24_sRGB */ {.format = GL_BGR, .internal_format = GL_SRGB8, .type = GL_INT},

		/* RGBA32, RGBA32_UNorm */ {.format = GL_RGBA, .internal_format = GL_RGBA8, .type = GL_UNSIGNED_BYTE},
		/* RGBA32_SNorm */ {.format = GL_RGBA, .internal_format = GL_RGBA8_SNORM, .type = GL_BYTE},
		/* RGBA32_UInt */ {.format = GL_RGBA, .internal_format = GL_RGBA8UI, .type = GL_UNSIGNED_INT},
		/* RGBA32_SInt */ {.format = GL_RGBA, .internal_format = GL_RGBA8I, .type = GL_INT},
		/* RGBA32_sRGB */ {.format = GL_RGBA, .internal_format = GL_SRGB8_ALPHA8, .type = GL_INT},

		/* BGRA32, BGRA32_UNorm */ {.format = GL_BGRA, .internal_format = GL_RGBA8, .type = GL_UNSIGNED_BYTE},
		/* BGRA32_SNorm */ {.format = GL_BGRA, .internal_format = GL_RGBA8_SNORM, .type = GL_BYTE},
		/* BGRA32_UInt */ {.format = GL_BGRA, .internal_format = GL_RGBA8UI, .type = GL_UNSIGNED_INT},
		/* BGRA32_SInt */ {.format = GL_BGRA, .internal_format = GL_RGBA8I, .type = GL_INT},
		/* BGRA32_sRGB */ {.format = GL_BGRA, .internal_format = GL_SRGB8_ALPHA8, .type = GL_INT},
		/* RGBA64_SFloat */{.format = GL_RGBA, .internal_format = GL_RGBA16F, .type = GL_FLOAT},
		/* RGBA128_SFloat */{.format = GL_RGBA, .internal_format = GL_RGBA32F, .type = GL_FLOAT},

		/* Depth16_UNorm */ {.format = GL_DEPTH_COMPONENT, .internal_format = GL_DEPTH_COMPONENT16, .type = GL_UNSIGNED_BYTE},
		/* Depth32_UNorm */ {.format = GL_DEPTH_COMPONENT, .internal_format = GL_DEPTH_COMPONENT32, .type = GL_UNSIGNED_BYTE}
	}};

	constexpr FormatData get_format_data(ImageFormat format)
	{
		return internal_fmts[static_cast<int>(format)];
	}
}

#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_IMAGE_FORMAT_HPP
