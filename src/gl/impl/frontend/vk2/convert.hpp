#ifndef TOPAZ_GL_IMPL_FRONTEND_VK2_CONVERT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK2_CONVERT_HPP
#if TZ_VULKAN
#include "gl/declare/texture.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"

namespace tz::gl
{
	constexpr vk2::ImageFormat to_vk2(TextureFormat fmt);
	constexpr TextureFormat from_vk2(vk2::ImageFormat fmt);
}

#include "gl/impl/frontend/vk2/convert.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK2_CONVERT_HPP
