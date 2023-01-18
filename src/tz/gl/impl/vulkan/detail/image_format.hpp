#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_FORMAT_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_FORMAT_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include <algorithm>
#include <array>
#include <span>

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_image
	 * Various image formats are supported. Note that PhysicalDevices may not support all formats listed here.
	 *
	 * image_formats are comprised of three properties.
	 *
	 * The enum values are named as `Components``Size``Internal` where:
	 * - Components (R, RG, RGB, BGR, RGBA): number of components and order.
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
	enum class image_format
	{
		/// - Undefined Format. It is mostly an error to use this.
		undefined = VK_FORMAT_UNDEFINED,
		
		R8 = VK_FORMAT_R8_UNORM,
		R8_UNorm = R8,
		R8_SNorm = VK_FORMAT_R8_SNORM,
		R8_UInt = VK_FORMAT_R8_UINT,
		R8_SInt = VK_FORMAT_R8_SINT,
		R8_sRGB = VK_FORMAT_R8_SRGB,
		
		R16 = VK_FORMAT_R16_UNORM,
		R16_UNorm = R16,
		R16_SNorm = VK_FORMAT_R16_SNORM,
		R16_UInt = VK_FORMAT_R16_UINT,
		R16_SInt = VK_FORMAT_R16_SINT,

		RG16 = VK_FORMAT_R8G8_UNORM,
		RG16_UNorm = RG16,
		RG16_SNorm = VK_FORMAT_R8G8_SNORM,
		RG16_UInt = VK_FORMAT_R8G8_UINT,
		RG16_SInt = VK_FORMAT_R8G8_SINT,
		RG16_sRGB = VK_FORMAT_R8G8_SRGB,

		RG32 = VK_FORMAT_R16G16_UNORM,
		RG32_UNorm = RG32,
		RG32_SNorm = VK_FORMAT_R16G16_SNORM,
		RG32_UInt = VK_FORMAT_R16G16_UINT,
		RG32_SInt = VK_FORMAT_R16G16_SINT,

		RGB24 = VK_FORMAT_R8G8B8_UNORM,
		RGB24_UNorm = RGB24,
		RGB24_SNorm = VK_FORMAT_R8G8B8_SNORM,
		RGB24_UInt = VK_FORMAT_R8G8B8_UINT,
		RGB24_SInt = VK_FORMAT_R8G8B8_SINT,
		RGB24_sRGB = VK_FORMAT_R8G8B8_SRGB,

		BGR24 = VK_FORMAT_B8G8R8_UNORM,
		BGR24_UNorm = BGR24,
		BGR24_SNorm = VK_FORMAT_B8G8R8_SNORM,
		BGR24_UInt = VK_FORMAT_B8G8R8_UINT,
		BGR24_SInt = VK_FORMAT_B8G8R8_SINT,
		BGR24_sRGB = VK_FORMAT_B8G8R8_SRGB,

		RGBA32 = VK_FORMAT_R8G8B8A8_UNORM,
		RGBA32_UNorm = RGBA32,
		RGBA32_SNorm = VK_FORMAT_R8G8B8A8_SNORM,
		RGBA32_UInt = VK_FORMAT_R8G8B8A8_UINT,
		RGBA32_SInt = VK_FORMAT_R8G8B8A8_SINT,
		RGBA32_sRGB = VK_FORMAT_R8G8B8A8_SRGB,

		BGRA32 = VK_FORMAT_B8G8R8A8_UNORM,
		BGRA32_UNorm = BGRA32,
		BGRA32_SNorm = VK_FORMAT_B8G8R8A8_SNORM,
		BGRA32_UInt = VK_FORMAT_B8G8R8A8_UINT,
		BGRA32_SInt = VK_FORMAT_B8G8R8A8_SINT,
		BGRA32_sRGB = VK_FORMAT_B8G8R8A8_SRGB,

		RGBA64_SFloat = VK_FORMAT_R16G16B16A16_SFLOAT,
		RGBA128_SFloat = VK_FORMAT_R32G32B32A32_SFLOAT,

		Depth16_UNorm = VK_FORMAT_D16_UNORM,
		Depth32_SFloat = VK_FORMAT_D32_SFLOAT
	};

	/*
	 * https://www.khronos.org/registry/vulkan/specs/1.2/pdf/vkspec.pdf
	 * pg 1081-1090 (big tables of mandatory supported formats)
	 */
	constexpr image_format safe_colour_attachment_formats[]
	{
		image_format::RGBA32_UInt,
		image_format::RGBA32_SInt,
		image_format::RGBA32_sRGB,

		image_format::BGRA32,
		image_format::BGRA32_sRGB,

		image_format::RG16_UInt,
		image_format::RG16_SInt,

		image_format::RG32_UInt,
		image_format::RG32_SInt,
	};

	constexpr image_format safe_depth_attachment_formats[]
	{
		image_format::Depth16_UNorm
	};

	constexpr image_format safe_sampled_image_formats[]
	{
		image_format::R8,
		image_format::R8_SNorm,
		image_format::R8_UInt,
		image_format::R8_SInt,

		image_format::R16_UInt,
		image_format::R16_SInt,

		image_format::RG16,
		image_format::RG16_SNorm,
		image_format::RG16_UInt,
		image_format::RG16_SInt,

		image_format::RG32_UInt,
		image_format::RG32_SInt,

		image_format::RGBA32,
		image_format::RGBA32_SNorm,
		image_format::RGBA32_UInt,
		image_format::RGBA32_SInt,
		image_format::RGBA32_sRGB,

		image_format::BGRA32,
		image_format::BGRA32_sRGB
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Meta information about image_formats.
	 */
	namespace format_traits
	{
		/**
		 * Retrieve a span of all image_formats which are guaranteed to be supported for a colour attachment for any framebuffer.
		 * An @ref Image can safely use these as a @ref Framebuffer colour attachment format without ensuring its corresponding @ref PhysicalDevice supports it via @ref PhysicalDevice::supports_image_colour_format.
		 */
		constexpr std::span<const image_format> get_mandatory_colour_attachment_formats()
		{
			return {safe_colour_attachment_formats};
		}

		/**
		 * Retrieve a span of all image_formats which are guaranteed to be supported for a depth attachment for any framebuffer.
		 * An @ref Image can safely use these as a @ref Framebuffer depth attachment format without ensuring its corresponding @ref PhysicalDevice supports it via @ref PhysicalDevice::supports_image_depth_format.
		 */
		constexpr std::span<const image_format> get_mandatory_depth_attachment_formats()
		{
			return {safe_depth_attachment_formats};
		}
		
		/**
		 * Retrieve a span of all image_formats which are guaranteed to be supported for a sampled image within a shader resource.
		 * An @ref Image can safely use these as a shader resource image format without ensuring its corresponding @ref PhysicalDevice supports it via @ref PhysicalDevice::supports_image_sampled_format.
		 */
		constexpr std::span<const image_format> get_mandatory_sampled_image_formats()
		{
			return {safe_sampled_image_formats};
		}

	}
}
#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_FORMAT_HPP
