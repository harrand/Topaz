#ifndef TOPAZ_GL_IMPL_FRONTEND_VK2_CONVERT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK2_CONVERT_HPP
#if TZ_VULKAN
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/impl/vulkan/detail/image_format.hpp"

namespace tz::gl
{
	constexpr tz::gl::vk2::ImageFormat to_vk2(ImageFormat fmt);
	constexpr ImageFormat from_vk2(tz::gl::vk2::ImageFormat fmt);
}

#include "tz/gl/impl/vulkan/convert.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK2_CONVERT_HPP
