#ifndef TOPAZ_GL_IMPL_FRONTEND_VK_CONVERT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK_CONVERT_HPP
#if TZ_VULKAN
#include "gl/declare/texture.hpp"
#include "gl/impl/backend/vk/image.hpp"

namespace tz::gl
{
    constexpr vk::Image::Format to_vk(TextureFormat fmt);
    constexpr TextureFormat from_vk(vk::Image::Format fmt);
}

#include "gl/impl/frontend/vk/convert.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK_CONVERT_HPP